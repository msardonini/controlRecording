/**
 * @file TestTcp.cpp
 * @brief Definition file.
 *
 * @author Igor Janjic
 * @date 01/30/2018
 */

#include "TestTcp.h"


const std::string TestTcp::tcpClientAddress = "";
const std::string TestTcp::tcpServerAddress = "";
const int TestTcp::tcpPort = 4003;

TestTcp::TestTcp()
    : tcpServer(nullptr),
      tcpClient(nullptr),
      task1ShouldExit(false),
      task1Executed(false)
{}

TestTcp::~TestTcp() {}

void TestTcp::SetUp()
{
    // Create the server
    this->task1 = std::bind(&TestTcp::serverTask1, this, std::placeholders::_1); // TODO: how is this done with a lambda function?
    this->tcpServer = new TcpServer(this->task1, this->tcpServerAddress, this->tcpPort, 1.0, 5.0);
}

void TestTcp::TearDown()
{
    this->tcpServer->disconnect();
    if (this->tcpServer)
        delete this->tcpServer;
}


bool TestTcp::serverTask1(int sockClient)
{
    std::cout << "Server executing task" << std::endl;
    while (!this->task1ShouldExit);
    std::cout << "Server completed task" << std::endl;
    this->task1Executed = true;
    return true;
}


TEST(GlobalTest, TestTcpServerDefaultConstructor)
{
    TcpServer* tcpServer = new TcpServer(); 
    ASSERT_EQ(tcpServer->sockServer, -1);
    ASSERT_EQ(tcpServer->sockClient, -1);
    ASSERT_EQ(tcpServer->port, 0);
    ASSERT_FALSE(tcpServer->serverAlive);
    ASSERT_FALSE(tcpServer->clientAlive);
    ASSERT_FALSE(tcpServer->time2Exit);
    delete tcpServer;
}


TEST_F(TestTcp, TestTcpServerConstructor)
{
    ASSERT_TRUE(this->tcpServer->serverAlive);
    ASSERT_TRUE(this->tcpServer->running);
    ASSERT_FALSE(this->tcpServer->time2Exit);
    ASSERT_FALSE(this->tcpServer->clientAlive);

    // Create the client and wait
    sleep(1);  // wait for server to connect
    this->tcpClient = new TcpClient(this->tcpClientAddress, this->tcpPort);
    sleep(1);  // wait for client to connect

    ASSERT_TRUE(this->tcpServer->clientAlive);
    ASSERT_STREQ(this->tcpServer->addressClient.c_str(), "localhost");
    this->task1ShouldExit = true;
    sleep(1);  // wait for task to exit
    ASSERT_TRUE(this->task1Executed);

    // Stop it
    ASSERT_TRUE(this->tcpServer->disconnect());
    sleep(1);  // wait for server to disconnect
    ASSERT_FALSE(this->tcpServer->running);

    delete this->tcpClient;
}


TEST_F(TestTcp, TestTcpServerReconnect)
{
    // Create the client then stop the server once done
    sleep(1);  // wait for server to connect
    this->tcpClient = new TcpClient(this->tcpClientAddress, this->tcpPort);
    sleep(1);  // wait for client to connect
    this->task1ShouldExit = true;
    sleep(1);  // wait for task to exit
    ASSERT_TRUE(this->tcpServer->disconnect());
    sleep(1);  // wait for server to disconnect
    this->task1ShouldExit = false;

    // Bring the server back online
    ASSERT_TRUE(this->tcpServer->connect(this->tcpServerAddress, this->tcpPort));
    ASSERT_TRUE(this->tcpServer->runInThread(this->task1, 1.0, 5.0));
    sleep(1);
    delete this->tcpClient;
    this->tcpClient = new TcpClient(this->tcpClientAddress, this->tcpPort);
    sleep(1);
    this->task1ShouldExit = true;
    sleep(1);
    ASSERT_TRUE(this->task1Executed);

    delete this->tcpClient;
}


/** Application main entry point.
 */
int main(int argc, char* argv[])
{
    // Initiate testing
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

