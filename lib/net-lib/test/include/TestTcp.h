/**
 * @file TestTcp.h
 * @brief Tests the TCP client and server classes.
 *
 * @author Igor Janjic
 * @date 01/30/2018
 */


#ifndef TEST_TCP_H
#define TEST_TCP_H

#include <functional>
#include <pthread.h>
#include <unistd.h>

// GTest
#include <gtest/gtest.h>

// Ours
#include "TcpClient.h"
#include "TcpServer.h"


/** Fixture for global tests */
class TestTcp : public ::testing::Test
{

public:

    /** Task that will be executed by the TcpServer.
     */
    bool serverTask1(int sockClient);

protected:

    /** Default constructor.
     */
    TestTcp();
  

    /** Default destructor.
     */
    virtual ~TestTcp();


    /** Code here will be called immediately after the constructor (right
     *  before each test).
     */
    virtual void SetUp();


    /** Code here will be called immediately after each test (right
     *  before the destructor).
     */
    virtual void TearDown();

    // Function pointer to task 1.
    std::function<bool(int)> task1;

    // Flag for whether task1 should exit.
    bool task1ShouldExit;

    // Flag for whether task1 executed successfully.
    bool task1Executed;

    // TcpServer that will be tested.
    TcpServer* tcpServer;

    // TcpClient that will be tested.
    TcpClient* tcpClient;

    // Address of the client is localhost.
    static const std::string tcpClientAddress;  

    // Address of the server is localhost.
    static const std::string tcpServerAddress;

    // Port is 4000.
    static const int tcpPort;

};  // TEST_TCP


#endif  // TEST_TCP_H

