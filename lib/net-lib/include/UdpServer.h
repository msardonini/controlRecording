/** 
 * @file UdpServer.h
 * @brief Creates a simple single client UDP server at a given address and port.
 * @author Igor Janjic
 * @date 02/17/2018
 *
 * Updates:
 * 02/17/2018 [janjic.igor] Created file
 */

#ifndef UDP_SERVER_H
#define UDP_SERVER_H

// STL
#include <cstring>
#include <string>
#include <iostream>
#include <functional>
#include <unistd.h>

// Network
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

// Thread
#include <pthread.h>

#ifdef WITH_TESTING
    #include <gtest/gtest_prod.h>
#endif

#include "Networking.h"


/** Trampoline function for starting the running thread.
 */
static void* udpServerRunTrampoline(void* args);


/** UdpServer encapsulates a UDP server which is responsible for receiving data
 *  streams from clients.
 *
 *  The UdpServer class is created by passing in a function pointer to a task,
 *  which processes a received UDP message, as well as the port to begin
 *  listening to connections on.
 *
 *  The task must be a function which accepts as a parameter the file
 *  descriptor of the server, a buffer containing the received data, the size
 *  of the buffer, and returns a boolean indicating whether or not the function
 *  completed succesfully.
 */
class UdpServer
{

public:

    // Arguments to the thread.
    struct ThreadArgs
    {
        UdpServer* thisPtr;
        std::function<bool(int, char*, size_t)> task;
        int readSize;
        double timeoutRead;
    };


    /** Default constructor.
     *
     *  User needs to manually connect and begin running. Good for handling
     *  errors.
     */
    UdpServer();


    /** Constructor.
     *
     *  Establishes the connection and begins running immediately (in a thread).
     *
     *  @param[in] task         Function pointer to a task.
     *  @param[in] address      Address to bind to. Use this parameter if you
     *                          want to bind to a particular interface,
     *                          otherwise leave it blank if you want to accept
     *                          connections from any interface.
     *  @param[in] port         Port to accept connections from.
     *  @param[in] readSize     Max size of reads.
     *  @param[in] recvBuffSize Size of the receive buffer.
     *  @param[in] timeoutRead  Timeout to use when reading.
     *  @param[in] mutlicast    Sets an option to reuse addressing.
     */
    UdpServer(std::function<bool(int, char*, size_t)> task,
        const std::string& address, int port, int readSize, int recvBuffSize,
        double timeoutRead, bool multicast = false);


    /** Destructor.
     */
    ~UdpServer();


    /** Connects to the socket.
     *
     *  @param[in] address      Address to connect to.
     *  @param[in] port         Port to connect to.
     *  @param[in] recvBuffSize Size of the receive buffer.
     *  @param[in] multicast    Sets an option to reuse addressing.
     *  @return                 True if the connection was established.
     */
    bool connect(const std::string& address, int port, int recvBuffSize,
        bool multicast = false);


    /** Stops running and disconnects the server.
     */
    bool disconnect();


    /** Runs the server.
     *
     *  The server starts listening over the socket for incoming messages. Once
     *  a UDP message is received, the server runs the function pointer task.
     *
     *  The server will run until the time2Exit flag is true. In order to be
     *  able to set the flag, you will need to run this function in a thread.
     *  Alternatively, use runInThread().
     *
     *  @param[in] task         Task to execute once a client connects.
     *  @param[in] readSize     Max size of reads.
     *  @param[in] timeoutRead  Timeout to use when reading.
     */
    void run(std::function<bool(int, char*, size_t)> task, int readSize, double timeoutRead);


    /** Runs the server in a thread.
     *
     *  The server creates a thread which calls the run function above.
     *
     *  @param[in] task         Task to execute once a client connects.
     *  @param[in] readSize     Max size of reads.
     *  @param[in] timeoutRead  Timeout to use when reading.
     *  @return                 True if the thread was succesfully created.
     */
    bool runInThread(std::function<bool(int, char*, size_t)> task, int readSize, double timeoutRead);

    
    /** Sends data to the server.
     */
    ssize_t send(const char* buff, size_t length);


    ssize_t receiveUdp(void* buf, int readSize_);


    void setClientInfo(std::string address_, int port_);

    /** Determines if the connection to the server is currently alive.
     *
     *  @return True if the connection to the server is alive.
     */
    bool isServerAlive() const;


    /** Determines if the server is currently running.
     */
    bool isRunning() const;


    /** Gets the file descriptor belonging to the server.
     *
     *  @return Server file descriptor.
     */
    int getServer() const;


    /** Gets the address of the client which sent the last message.
     *
     *  @return Client address string.
     */
    std::string getClientAddress() const;


private:

    // Buffer containing the last read message.
    char* buff;

    // Max size of reads.
    int readSize;

    // Timeout to use when reading.
    double timeoutRead;

    // Socket file descriptor to the server.
    int sockServer;

    // Struct containing the server address information
    struct sockaddr_in server;

    // Struct containing the client address information.
    struct sockaddr_in client;

    // Address of the interface we are binding to.
    std::string addressServer;

    // Address of the client we are connected to.
    std::string addressClient;

    // Port we are receiving connections from.
    int port;

    // Function to execute when a client connects.
    std::function<bool(int, char*, size_t)> task;

    // Whether the connection to the server is currently alive.
    bool serverAlive;

    // Whether the run loop started.
    bool running;

    // Whether it is time to exit the run loop.
    bool time2Exit;

    // Thread ID.
	pthread_t tid;

    // Thread arguments
    ThreadArgs threadArgs;

    #ifdef WITH_TESTING
        friend class TestUdp;
    #endif

};  // UDP_SERVER


#endif  // UDP_SERVER_H
