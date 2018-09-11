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

//Ours
#include "UdpServer.h"
#include "messageStructure.h"

	
class remoteSender
{
public:
	
	remoteSender(std::string ipAddrHost);


	~remoteSender();

	int run();

	int createSendMessage();

	int onMessageReceived();

	//Functions to control IO with onboard LED lights
	int setLedFlashing();
	int setLedSolid();
	int setLedRecording();
	int setLedRecordingOff();


private:
	//Simple bool to show if object is running
	bool isRunning;
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











#endif //REMOTESENDER_H