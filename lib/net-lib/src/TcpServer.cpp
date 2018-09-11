/** 
 * @file TcpServer.cpp
 * @brief Creates a simple single client TCP server at a given address and port.
 * @author Igor Janjic
 * @date 01/30/2018
 *
 * Updates:
 * 01/30/2018 [janjic.igor] Created file
 */

#include "TcpServer.h"


void* tcpServerRunTrampoline(void* args)
{
    TcpServer::ThreadArgs* threadArgs = (TcpServer::ThreadArgs*)args;
	threadArgs->thisPtr->run(threadArgs->task, threadArgs->timeoutClientAccept, threadArgs->timeoutClientBoot);
    return NULL;
}


TcpServer::TcpServer()
    : timeoutClientAccept(0),
      timeoutClientBoot(0),
      sockServer(-1),
      sockClient(-1),
      addressServer(""),
      addressClient(""),
      port(0),
      serverAlive(false),
      clientAlive(false),
      running(false),
      time2Exit(false),
      tid(-1) {}


TcpServer::TcpServer(std::function<bool(int)> task_, const std::string& address_,
    int port_, double timeoutClientAccept_, double timeoutClientBoot_,
    bool multicast_)
    : timeoutClientAccept(0),
      timeoutClientBoot(0),
      sockServer(-1),
      sockClient(-1),
      addressServer(""),
      addressClient(""),
      port(0),
      serverAlive(false),
      clientAlive(false),
      running(false),
      time2Exit(false),
      tid(-1)
{
    if (this->connect(address_, port_, multicast_))
        this->runInThread(task_, timeoutClientAccept_, timeoutClientBoot_);
    else
        std::cerr << "Could not establish a connection on '" << address_ << "' port '" << port_ << "'" << std::endl;
}


bool TcpServer::connect(const std::string& address_, int port_, bool multicast_)
{
    this->addressServer = address_;
    this->port = port_;

    // Allow localhost strings
    if (this->addressServer == "localhost")
        this->addressServer = "127.0.0.1";

    // Create the socket
    this->sockServer = ::socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockServer == -1)
    {
        std::cerr << "Could not connect socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Set option to reuse addressing, if failure not a fatal error
    int reuse = 1;
    if (::setsockopt(this->sockServer, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse, (socklen_t)sizeof(reuse)) == -1)
    {
        std::cerr << "Could not set otpion to reuse addressing" << std::endl;
    }

    memset(&this->server, 0, sizeof(this->server));
    memset(&this->client, 0, sizeof(this->client));

    // Construct the server address struct
    this->server.sin_family=AF_INET;
    this->server.sin_port=htons(port);

    // Pay special attention to the address to set it up right
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

    // Try to bind
    if (::bind(this->sockServer, (struct sockaddr*)&this->server, sizeof(this->server)) == -1)
    {
        std::cerr << "Could not bind: " << strerror(errno) << std::endl;
        return false;
    }

    // Try to listen
    if (::listen(this->sockServer, 1) == -1)
    {
        std::cerr << "Could not listen: " << strerror(errno) << std::endl;
        return false;
    }

    this->serverAlive = true;
    return true;
}


bool TcpServer::disconnect()
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
        if (!Networking::flushSocket(this->sockServer, 2.0))
            std::cerr << "Warning: cannot flush socket: " << strerror(errno) << std::endl;   
        if (::close(this->sockServer) == -1)
        {
            std::cerr << "Could not close server socket: " << strerror(errno) << std::endl;
            this->sockServer = -1;
            return false;
        }
        this->sockServer = -1;
    }

    this->serverAlive = false;
    this->clientAlive = false;

    return true;
}


void TcpServer::run(std::function<bool(int)> task_, double timeoutClientAccept_, double timeoutClientBoot_)
{
    this->task = task_;
    this->timeoutClientAccept = timeoutClientAccept_;
    this->timeoutClientBoot = timeoutClientBoot_;

    while (!time2Exit)
    {
        socklen_t csSize = sizeof(this->client);
        //std::cout << "Server waiting for connections" << std::endl;

        fd_set set;
        struct timeval tv;
        int rv;
        FD_ZERO(&set); /* clear the set */
        FD_SET(this->sockServer, &set); /* add our file descriptor to the set */

        tv.tv_sec  = (long)timeoutClientAccept;
        tv.tv_usec = (long)((timeoutClientAccept - tv.tv_sec) * 1000000);

        rv = select(this->sockServer + 1, &set, NULL, NULL, &tv);
        if (rv == -1)
        {
            std::cerr << "Error on select" << std::endl;
            continue;
        }
        else if (rv == 0)
        {
            //std::cerr << "Timeout occured" << std::endl;
            continue;
        }
        else
        {
            this->sockClient = ::accept(this->sockServer, (struct sockaddr *)&this->client, &csSize);
            if (this->sockClient < 0)
            {
                std::cerr << "Failed to accept client" << std::endl;
                continue;
            }

            // Figure out who we are connected to    
            struct hostent* hostp;
            hostp = ::gethostbyaddr((const char *)&this->client.sin_addr.s_addr, sizeof(this->client.sin_addr.s_addr), AF_INET);
            if (hostp == NULL) 
            {
                char* hostaddrp = inet_ntoa(this->client.sin_addr);
                if (hostaddrp == NULL)
                {
                    std::cerr << "Could not convert host address to a string" << std::endl;
                    continue;
                }
                else
                    this->addressClient = hostaddrp;
            }
            else
                this->addressClient = hostp->h_name;

            //std::cout << "Server established connection with '" << this->addressClient << "'" << std::endl;
            this->clientAlive = true;

            // Process this client
            if (!task(this->sockClient))
            {
                std::cerr << "Error in client task" << std::endl;
            }

            // Once the task is done, close the socket to the client.
            if (!Networking::flushSocket(this->sockClient, 2.0))
                std::cerr << "Warning: cannot flush socket" << std::endl;   
            if (::close(this->sockClient) == -1)
                std::cerr << "Could not close client socket: " << strerror(errno) << std::endl;

            this->addressClient = "";
            this->clientAlive = false;

        }
    }
    this->running = false;
}


bool TcpServer::runInThread(std::function<bool(int)> task_, double timeoutClientAccept_, double timeoutClientBoot_)
{
    this->threadArgs.thisPtr = this;
    this->threadArgs.task = task_;
    this->threadArgs.timeoutClientAccept = timeoutClientAccept_;
    this->threadArgs.timeoutClientBoot = timeoutClientBoot_;
    int result = pthread_create(&this->tid, NULL, &tcpServerRunTrampoline, &threadArgs);
    if (result)
        return false;

    this->running = true;
    return true;
}


bool TcpServer::isServerAlive() const
{
    return this->serverAlive;
}


bool TcpServer::isClientAlive() const
{
    return this->clientAlive;
}


int TcpServer::getServer() const
{
    return this->sockServer;
}


int TcpServer::getClient() const
{
    return this->sockClient;
}

