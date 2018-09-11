/** 
 * @file TcpServer.h
 * @brief Creates a simple single client TCP server at a given address and port.
 * @author Igor Janjic
 * @date 01/30/2018
 *
 * Updates:
 * 01/30/2018 [janjic.igor] Created file
 */

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

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
static void* tcpServerRunTrampoline(void* args);


/** TcpServer encapsulates TCP connections.
 *
 *  The TcpServer class is created by passing in a function pointer to a task,
 *  which processes a client, the address of the address to bind to, the port to
 *  begin listening to connections on, and various timeout configuration
 *  parameters.
 *
 *  The task must be a function which accepts as a parameter the socket file
 *  descriptor, and returns a boolean indicating whether or not the function
 *  completed succesfully.
 */
class TcpServer
{

public:

    // Arguments to the thread.
    struct ThreadArgs
    {
        TcpServer* thisPtr;
        std::function<bool(int)> task;
        double timeoutClientAccept;
        double timeoutClientBoot;
    };

    /** Default constructor.
     *
     *  User needs to manually connect and begin running. Good for handling
     *  errors.
     */
    TcpServer();


    /** Constructor.
     *
     *  Establishes the connection and begins running immediately (in a thread).
     *
     *  @param[in] task     Function pointer to a task.
     *  @param[in] address  Address to bind to. Use this parameter if you want
     *                      to bind to a particular interface, otherwise leave
     *                      it blank if you want to accept connections from any
     *                      interface.
     *  @param[in] port     Port to accept connections from.
     *  @param[in] timeoutClientAccept  Timeout to use when accepting
     *                                  connections to clients.
     *  @param[in] timeoutClientBoot    Timeout to use when booting unresponsive
     *                                  clients.
     *  @param[in] mutlicast    Sets an option to reuse addressing.
     */
    TcpServer(std::function<bool(int)> task, const std::string& address,
        int port, double timeoutClientAccept = 0, double timeoutClientBoot = 0,
        bool multicast = false);


    /** Connects to the socket.
     *
     *  @param[in] address      Address of the server.
     *  @param[in] port         Port to connect to.
     *  @param[in] multicast    Whether or not to reuse addressing (multicast).
     *  @return                 True if the connection was established.
     */
    bool connect(const std::string& address, int port, bool multicast = false);


    /** Stops running and disconnects the server.
     */
    bool disconnect();


    /** Runs the server.
     *
     *  The server begins waiting for a client to connect. Once a client
     *  connects, the server spawns a thread to handle the client by running
     *  the function pointer task.
     *
     *  The server will run until the time2Exit flag is true. In order to be
     *  able to set the flag, you will need to run this function in a thread.
     *  Alternatively, use runInThread().
     *
     *  @param task                     Task to execute once a client connects.
     *  @param[in] timeoutClientAccept  Timeout to use when accepting
     *                                  connections to clients.
     *  @param[in] timeoutClientBoot    Timeout to use when booting unresponsive
     *                                  clients.
     */
    void run(std::function<bool(int)> task, double timeoutClientAccept, double timeoutClientBoot);


    /** Runs the server in a thread.
     *
     *  The server creates a thread which calls the run function above.
     *
     *  @param[in] task                 Task to execute once a client connects.
     *  @param[in] timeoutClientAccept  Timeout to use when accepting
     *                                  connections to clients.
     *  @param[in] timeoutClientBoot    Timeout to use when booting unresponsive
     *                                  clients.
     *  @return                         True if the thread was succesfully
     *                                  created.
     */
    bool runInThread(std::function<bool(int)> task, double timeoutClientAccept, double timeoutClientBoot);


    /** Determines if the connection to the server is currently alive.
     *
     *  @return True if the connection to the server is alive.
     */
    bool isServerAlive() const;


    /** Determines if the connection to the client is currently alive.
     * 
     *  @return True if the connection to the client is alive.
     */
    bool isClientAlive() const;


    /** Gets the file descriptor belonging to the server.
     *
     *  @return Server file descriptor.
     */
    int getServer() const;


    /** Gets the file descriptor belonging to the currently connected client.
     *
     *  @return Client file descriptor.
     */
    int getClient() const;


private:

    // Timeout to use when accepting connections to clients.
    double timeoutClientAccept;

    // Timeout to use before booting clients that aren't responsive
    double timeoutClientBoot;

    // Socket file descriptor of the server.
    int sockServer;

    // Socket file descriptor of the client.
    int sockClient;

    // Function to execute when a client connects.
    std::function<bool(int)> task;

    // Struct containing the address information of the server.
    struct sockaddr_in server;

    // Struct containing the address information of the client.
    struct sockaddr_in client;
    
    // Address of the interface we are binding to.
    std::string addressServer;

    // Address of the client we are connected to.
    std::string addressClient;

    // Port we are receiving connections on.
    int port;

    // Whether the connection to the server is currently alive.
    bool serverAlive;

    // Whether the connection to the client is currently alive.
    bool clientAlive;

    // Whether the run loop started.
    bool running;

    // Whether it is time to exit the run loop.
    bool time2Exit;

    // Thread ID.
	pthread_t tid;

    // Thread arguments
    ThreadArgs threadArgs;

    #ifdef WITH_TESTING
        friend class TestTcp;
        FRIEND_TEST(GlobalTest, TestTcpServerDefaultConstructor);
        FRIEND_TEST(TestTcp, TestTcpServerConstructor);
    #endif

};  // TCP_SERVER


#endif  // TCP_SERVER_H
