/**
 * @file remoteSender.h
 * @brief Defines the remote sender class
 *
 * @author Mike Sardonini
 * @date 09/10/2018
 */

#ifndef HOSTRECEIVER_H
#define HOSTRECEIVER_H

//System Includes
#include <string.h>
#include <thread>
#include <memory>
#include <termios.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

//Ours
#include "UdpServer.h"
#include "messageStructure.h"


enum HOST_STATES_t
{
	STANDBY,
	RECORDING
};


class hostReceiver
{
public:
	
	//Bluetooth constructor
	hostReceiver();
	//UDP constructor
	hostReceiver(std::string ipAddrHost);


	~hostReceiver();

	//Threads that monintor reads and writes from network interfaces
	int readThread();
	int writeThread();

	/** Function that reads data from the other device */
	ssize_t receiveData();

	//Thread the monitors the states of the buttons to issue commands
	int buttonThread();

	int createSendMessage();

	int onMessageReceived();


	int stopRecording();
	int startRecording();

	// Get the current time in microseconds
	uint64_t getTimeUsec();


private:
	//Simple bool to show if object is running
	bool isRunning;
	bool useBluetooth;
	bool useUDP;

	int fd; //file descriptor for bluetooth serial

	//Enum to describe what state the program is reading from the host
	enum HOST_STATES_t commandedState;

	//Enum to describe what state the program is commanding to the Host
	enum HOST_STATES_t hostState;

	//threads to handle the incoming and outgoing of messages
	std::thread readThread_h;
	std::thread writeThread_h;

	//Current state of this program
	uint64_t timestamp_us;
	uint8_t mode;

	//Variables to handle message connections with the host
	uint64_t lastTimestampReceived_us;
	uint64_t lastModeReceived;

	//Object for handling the sending and reading of UDP packets
	UdpServer server;

	messageStructure_t sndMessage;
	uint8_t sndbuf[256];
	
	messageStructure_t rcvMessage;
	uint8_t rcvbuf[256];

protected:

};


#endif //HOSTRECEIVER_H