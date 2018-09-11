/** 
 * @file TcpClient.h
 * @brief Creates a simple TCP client connection to a given address and port.
 * @author Igor Janjic
 * @date 01/30/2018
 *
 * Updates:
 * 01/30/2018 [janjic.igor] Created file
 */

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

// STL
#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>

// Network
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "Networking.h"


/** TcpClient encapsulates a TCP client connection to a given address and port.
 */
class TcpClient
{

public:

    /** Default constructor.
     *
     *  @param[in] address  Address of the server. If an empty string, assumes
     *                      localhost.
     *  @param[in] port     Port of the server.
     */
    TcpClient(const std::string& address, int port);


    /** Destructor.
     *
     *  Disconnects from the socket.
     */
    ~TcpClient();


    /** Connects to the socket.
     *
     *  @param[in] address  Address of the server.
     *  @param[in] port     Port of the server.
     *  @return             True if the connection was established.
     */
    bool connect(const std::string& address, int port);


    /** Disconnects from the socket.
     *
     *  Clears the address and port and unsets the alive flag.
     */
    bool disconnect();


    /** Determines if the connection is currently alive.
     *
     *  @return True if the connection is alive.
     */
    bool isAlive() const;

private:

    /** Constructs the server struct.
     *
     *  @param[in] address  Address of the server.
     *  @param[in] port     Port of the server.
     */
    bool construct(const std::string& address, int port);

    // Socket file descriptor.
    int sock;

    // Address currently connected to.
    std::string address;

    // Port currently connected to.
    int port;

    // Struct representing server connection.
    struct sockaddr_in server;

    // Whether or not the connection is alive.
    bool alive;

    #ifdef WITH_TESTING
    #endif

};  // TCP_CLIENT


#endif  // TCP_CLIENT_H

