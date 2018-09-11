/** 
 * @file TcpClient.cpp
 * @brief Creates a simple TCP client connection to a given address and port.
 * @author Igor Janjic
 * @date 01/30/2018
 *
 * Updates:
 * 01/30/2018 [janjic.igor] Created file
 */

#include "TcpClient.h"


TcpClient::TcpClient(const std::string& address_, int port_)
    : sock(-1),
      address(""),
      port(0),
      alive(false)
{
    this->connect(address_, port_);
}


TcpClient::~TcpClient()
{
    this->disconnect();
}


bool TcpClient::connect(const std::string& address_, int port_)
{
    // Do not try to connect if a connection is already present
    if (this->sock == -1)
    {
        this->alive = true;
        this->port = port_;

        if (address_ == "" || address_ == "localhost")
            this->address = "127.0.0.1";
        else
            this->address = address_;

        this->sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (this->sock == -1)
        {
            std::cerr << "Could not connect socket: " << strerror(errno) << std::endl;
            this->alive = false;
        }
        else if (this->construct(this->address, port_))
        {
            //std::cout << "Client attempting to connect..." << std::endl;
            // If we got the hostname, go ahead and connect 
            if (::connect(this->sock, (struct sockaddr*)&(this->server), sizeof(this->server)) < 0)
            {
                std::cerr << "Connection failed: " << strerror(errno) << std::endl;
                this->alive = false;
            }
        }
    }
    else
        return false;

    return this->alive;
}


bool TcpClient::disconnect()
{
    if (this->sock != -1)
    {
        if (!Networking::flushSocket(this->sock, 2.0))
            std::cerr << "Warning: cannot flush socket: " << strerror(errno) << std::endl;   

        if (::close(this->sock) == -1)
        {
            std::cerr << "Could not close socket: " << strerror(errno) << std::endl;
            this->sock = -1;
            return false;
        }
        this->sock = -1;
    }
    this->alive = false;
    return true;
}


bool TcpClient::construct(const std::string& address_, int port_)
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


bool TcpClient::isAlive() const
{
    return this->alive;
}

