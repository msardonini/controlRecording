/**
 * @file remoteSender.h
 * @brief Defines the remote sender class
 *
 * @author Mike Sardonini
 * @date 09/10/2018
 */

#ifndef REMOTESENDER_H
#define REMOTESENDER_H

//System Includes
#include <string.h>
#include <thread>

//Ours
#include "UdpServer.h"
#include "messageStructure.h"

enum REMOTE_STATES_t
{
	DISCONNECTED,
	STANDBY,
	RECORDING
};

enum LED_COLORS_t
{
	RED,
	GREEN
};

enum LED_STATUS_t
{
	OFF,
	ON,
	FLASHING
};


class remoteSender
{
public:
	
	remoteSender(std::string ipAddrHost);


	~remoteSender();

	//Threads that monintor reads and writes from network interfaces
	int readThread();
	int writeThread();

	//Thread the monitors the states of the buttons to issue commands
	int buttonThread();

	int createSendMessage();

	int onMessageReceived();

	//Functions to control IO with onboard LED lights
	int LedControlThread(enum LED_COLORS_t);
	int setLedFlashing(enum LED_COLORS_t color);
	int setLedOn(enum LED_COLORS_t color);
	int setLedOff(enum LED_COLORS_t color);

	// Get the current time in microseconds
	uint64_t getTimeUsec();


private:
	//Simple bool to show if object is running
	bool isRunning;

	//Enum to describe what state the program is reading from the host
	enum REMOTE_STATES_t remoteState;

	//Enum to describe what state the program is commanding to the Host
	enum REMOTE_STATES_t hostState;

	//threads to handle the incoming and outgoing of messages
	std::thread readThread_h;
	std::thread writeThread_h;

	//Thread to handle the reads of button pushes
	std::thread buttonThread_h;

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

	//Thread to control the status of the LEDs
	std::thread redLedThread;
	std::thread greenLedThread;

	//LED status
	enum LED_STATUS_t redLedStatus;
	enum LED_STATUS_t greenLedStatus;

protected:

};











#endif //REMOTESENDER_H