/** 
 * @file UdpClient.cpp
 * @brief Creates a simple UDP client connection to a given address and port.
 * @author Igor Janjic
 * @date 01/30/2018
 *
 * Updates:
 * 01/30/2018 [janjic.igor] Created file
 */

#include "UdpClient.h"


UdpClient::UdpClient()
    : sock(-1),
      address(""),
      port(0),
      alive(false) {}


UdpClient::UdpClient(const std::string& address_, int port_)
    : sock(-1),
      address(""),
      port(0),
      alive(false)
{
    this->connect(address_, port_);
}


UdpClient::~UdpClient()
{
    this->disconnect();
}


bool UdpClient::connect(const std::string& address_, int port_)
{
    // Do not try to connect if a connection is already present
    if (this->sock == -1)
    {
        this->port = port_;

        if (address_ == "" || address_ == "localhost")
            this->address = "127.0.0.1";
        else
            this->address = address_;

        this->sock = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (this->sock == -1)
        {
            std::cerr << "Could not connect socket: " << strerror(errno) << std::endl;
            this->setAlive(false);
        }
        else
        {
            if (!this->construct(this->address, port_))
                std::cerr << "Could not construct server address" << std::endl;
            else
                this->setAlive(true);
        }
    }
    else
        return false;

    return this->isAlive();
}


bool UdpClient::disconnect()
{
    this->setAlive(false);

    if (this->sock != -1)
    {
        if (::close(this->sock) == -1)
        {
            std::cerr << "Could not close socket: " << strerror(errno) << std::endl;
            this->sock = -1;
            return false;
        }
        this->sock = -1;
    }
    return true;
}


bool UdpClient::construct(const std::string& address_, int port_)
{
    memset(&this->server, 0, sizeof(this->server));
    if (inet_addr(address_.c_str()) == -1)
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

    this->server.sin_family = AF_INET;
    this->server.sin_port = htons(port_);
    return true;
}


bool UdpClient::send(char* buff, size_t length)
{
    bool okay = true;
    if (this->isAlive())
    {
        int lenSent = sendto(this->sock, buff, length, 0, (struct sockaddr*)&this->server, sizeof(this->server));

        if (lenSent == -1)
        {
            std::cerr << "Failed to send: " << strerror(errno) << std::endl;
            return okay = false;
        }
        else if (lenSent != length)
        {
            std::cerr << "Could not send the entire message" << std::endl; 
        }
    }
    else
        okay = false;
    return okay;
}


bool UdpClient::isAlive()
{
    std::lock_guard<std::mutex> lock(this->mutexUdp);
    return this->alive;
}


void UdpClient::setAlive(bool alive_)
{
    std::lock_guard<std::mutex> lock(this->mutexUdp);
    this->alive = alive_;
}

