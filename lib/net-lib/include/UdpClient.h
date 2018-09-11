/** 
 * @file UdpClient.h
 * @brief Creates a simple UDP client connection to a given address and port.
 * @author Igor Janjic
 * @date 01/30/2018
 *
 * Updates:
 * 01/30/2018 [janjic.igor] Created file
 */

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

// STL
#include <mutex>
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


/** UdpClient encapsulates a UDP client connection to a given address and port
 *  which streams data to a UDP server.
 */
class UdpClient
{

public:

    /** Default constructor.
     *
     *  User needs to manually connect and begin running. Good for handling
     *  errors.
     */
    UdpClient();


    /** Default constructor.
     *
     *  @param[in] address  Address of the server. If an empty string, assumes
     *                      localhost.
     *  @param[in] port     Port of the server.
     */
    UdpClient(const std::string& address, int port);


    /** Destructor.
     *
     *  Disconnects from the socket.
     */
    ~UdpClient();


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


    /** Sends data to the server.
     */
    bool send(char* buff, size_t length);


    /** Determines if the connection is currently alive.
     *
     *  @return True if the connection is alive.
     */
    bool isAlive();

private:

    /** Sets the alive flag.
     */
    void setAlive(bool alive);


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

    // Mutex for making thread safe calls.
    std::mutex mutexUdp;

    #ifdef WITH_TESTING
    #endif

};  // UDP_CLIENT


#endif  // UDP_CLIENT_H

