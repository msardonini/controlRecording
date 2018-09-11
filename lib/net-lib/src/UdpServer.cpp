/** 
 * @file UdpServer.cpp
 * @brief Creates a simple single client UDP server at a given address and port.
 * @author Igor Janjic
 * @date 02/17/2018
 *
 * Updates:
 * 02/17/2018 [janjic.igor] Created file
 */

#include "UdpServer.h"


void* udpServerRunTrampoline(void* args)
{
    UdpServer::ThreadArgs* threadArgs = (UdpServer::ThreadArgs*)args;
	threadArgs->thisPtr->run(threadArgs->task, threadArgs->readSize, threadArgs->timeoutRead);
    return NULL;
}


UdpServer::UdpServer()
    : buff(nullptr),
      readSize(0),
      timeoutRead(0),
      sockServer(-1),
      addressServer(""),
      addressClient(""),
      port(0),
      serverAlive(false),
      running(false),
      time2Exit(false),
	  tid(-1) {}


UdpServer::UdpServer(std::function<bool(int, char*, size_t)> task_,
    const std::string& address_, int port_, int readSize_, int recvBuffSize_,
    double timeoutRead_, bool multicast_)
    : buff(nullptr),
      readSize(0),
      timeoutRead(0),
      sockServer(-1),
      addressServer(""),
      addressClient(""),
      port(0),
      serverAlive(false),
      running(false),
      time2Exit(false),
	  tid(-1)
{
    if (this->connect(address_, port_, recvBuffSize_, multicast_))
        this->runInThread(task_, readSize_, timeoutRead_);
    else
        std::cerr << "Could not establish a connection on '" << address_ << "' port '" << port_ << "'" << std::endl;
}


UdpServer::~UdpServer()
{
    if (this->buff)
        delete[] this->buff;

    this->disconnect();
}


bool UdpServer::connect(const std::string& address_, int port_, int recvBuffSize_, bool multicast_)
{
    this->addressServer = address_;
    this->port = port_;

    // Allow localhost strings
    if (this->addressServer == "localhost")
        this->addressServer = "127.0.0.1";

    memset(&this->server, 0, sizeof(this->server));
    memset(&this->client, 0, sizeof(this->client));

    // Create a udp socket
    if ((this->sockServer = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        std::cerr << "error: cannot create socket" << std::endl;
        return false;
    }
    else
    {
        // Set option to reuse addressing, if failure not a fatal error
        if (multicast_)
        {
            int reuse = 1;
            if (::setsockopt(this->sockServer, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == -1)
            {
                std::cerr << "Could not set option to reuse addressing" << std::endl;
            }
            if (::setsockopt(this->sockServer, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) == -1)
            {
                std::cerr << "Could not set option to reuse port" << std::endl;
            }

        }

        // Bind the socket to the IP address of the TX2 interface
        memset((char *)&this->server, 0, sizeof(this->server));
        this->server.sin_family = AF_INET;
        this->server.sin_port = htons(this->port);
        if (address_ == "")
            this->server.sin_addr.s_addr = htonl(INADDR_ANY);
        else if (inet_addr(address_.c_str()) == -1)
        {
            struct hostent* he;
            struct in_addr** addressList;
            if ((he = gethostbyname(address_.c_str())) == NULL)
            {
                herror("gethostbyname");
                std::cerr << "Failed to resolve hostname" << std::endl;
                return false;
            }
            else
                memcpy(&this->server.sin_addr, he->h_addr_list[0], he->h_length);
        }
        else
        {
            this->server.sin_addr.s_addr = inet_addr(address_.c_str());
        }

        if (::bind(this->sockServer, (struct sockaddr*)&this->server, sizeof(this->server)) < 0)
        {
            std::cerr << "Bind failed: " << strerror(errno) << std::endl;
            return false;
        }
    }

    // Increase the receive buffer's size
    // PROTIP: sudo sysctl -w net.core.rmem_max=<recvBuffSize>  # if reaching max
    if (recvBuffSize_ > 0)
    {
        if (::setsockopt(this->sockServer, SOL_SOCKET, SO_RCVBUF, (char*)&recvBuffSize_, sizeof(recvBuffSize_)) == -1)
            std::cerr << "error: could not set receive buffer size for socket: " << strerror(errno) << std::endl;
    }

    this->serverAlive = true;
    return true;
}

void UdpServer::setClientInfo(std::string address_, int port_)
{   
    this->client.sin_addr.s_addr = inet_addr(address_.c_str());
    this->client.sin_family = AF_INET;
    this->client.sin_port = htons(port_);
}

bool UdpServer::disconnect()
{
    if (this->running)
    {
        // Give the go-ahead to exit
	    this->time2Exit = true;
   
	    // Wait for exit
        void *returnValue;
	    pthread_join(this->tid, &returnValue);

        if ((intptr_t)returnValue)
        {
            //std::cerr << "Could not join thread: code: '" << (intptr_t)returnValue << "'";
            return false;
        }

        if (this->running)
        {
            std::cerr << "Thread still running" << std::endl;
            return false;
        }
        else
            time2Exit = false;
    }

    if (this->sockServer != -1)
    {
        //std::cout << "Disconnecting server" << std::endl;
        if (::close(this->sockServer) == -1)
        {
            std::cerr << "Could not close server socket: " << strerror(errno) << std::endl;
            this->sockServer = -1;
            return false;
        }
        this->sockServer = -1;
    }

    return true;
}

ssize_t UdpServer::receiveUdp(void* buf, int readSize_)
{
    socklen_t addrlen = sizeof(this->client);
    return recvfrom(this->sockServer, buf, readSize_, 0, (struct sockaddr *)&this->client, &addrlen);
}

void UdpServer::run(std::function<bool(int, char*, size_t)> task_, int readSize_, double timeoutRead_)
{
    this->task = task_;
    this->readSize = readSize_;
    this->timeoutRead = timeoutRead_;

    if (this->buff)
        delete[] this->buff;

    this->buff = new char[readSize_];

    // Set a timeout for reads
    if (this->timeoutRead > 0)
    {
        if (!Networking::setTimeoutReceive(this->sockServer, this->timeoutRead))
            std::cerr << "Could not set timeout on receive" << std::endl;
    }

    while (!time2Exit)
    {
        socklen_t addrlen = sizeof(this->client);
        size_t recvlen = recvfrom(this->sockServer, this->buff, this->readSize, 0, (struct sockaddr *)&this->client, &addrlen);

        if (recvlen != -1)
        {
            // Figure out who we are connected to    
            std::string hostaddr;
            char* hostaddrp = inet_ntoa(this->client.sin_addr);
            if (hostaddrp == NULL)
            {
                std::cerr << "Could not convert host address to a string" << std::endl;
                continue;
            }
            else
                hostaddr = hostaddrp;

            if (this->addressClient != hostaddr)
            {
                //std::cout << "Server established connection with '" << hostaddr << "'" << std::endl;
            }
            this->addressClient = hostaddr;

            if (!task(this->sockServer, this->buff, recvlen))
                std::cerr << "Error in client task" << std::endl;
        }
    }
    this->running = false;
}


bool UdpServer::runInThread(std::function<bool(int, char*, size_t)> task_, int readSize_, double timeoutRead_)
{
    this->threadArgs.thisPtr = this;
    this->threadArgs.task = task_;
    this->threadArgs.readSize = readSize_;
    this->threadArgs.timeoutRead = timeoutRead_;
    int result = pthread_create(&this->tid, NULL, &udpServerRunTrampoline, &threadArgs);
    if (result)
        return false;
    this->running = true;
    return true;
}


ssize_t UdpServer::send(const char* buff, size_t length)
{
    ssize_t lenSent;
    bool okay = true;
    if (this->serverAlive)
    {
        lenSent = sendto(this->sockServer, buff, length, 0, (struct sockaddr*)&this->client, sizeof(this->client));

        if ((lenSent == -1) || (lenSent != length))
        {
            std::cerr << "Failed to send: " << strerror(errno) << std::endl;
            // return okay = false;
        }
    }
    else
        okay = false;
    return lenSent;
}


bool UdpServer::isServerAlive() const
{
    return this->serverAlive;
}


bool UdpServer::isRunning() const
{
    return this->running;
}


int UdpServer::getServer() const
{
    return this->sockServer;
}


std::string UdpServer::getClientAddress() const
{
    return this->addressClient;
}

