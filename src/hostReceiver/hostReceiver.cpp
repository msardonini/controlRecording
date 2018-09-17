/**
 * @file hostReceiver.cpp
 * @brief Control the status of a video recorder on a headless machine using a remote raspberry pi
 *
 * @author Mike Sardonini
 * @date 09/16/2018
 */

#include "hostReceiver.h" 



/** Default Constructor

 */
hostReceiver::hostReceiver(std::string ipAddr)
	: commandedState(STANDBY),
	hostState(STANDBY),
	isRunning(true)
{
	//Port to read from the headless machine
	int portRemote = 200;
	int portHost = 201;

	this->server = UdpServer();

	//Connect to Any available local IP address and listen on given port 
	this->server.connect("", portHost, 256);
	this->server.setClientInfo(ipAddr, portRemote);

	this->readThread_h = std::thread(&hostReceiver::readThread, this);
	this->writeThread_h = std::thread(&hostReceiver::writeThread, this);
	
}

/** Default Destructor

 */
hostReceiver::~hostReceiver()
{


}

/** Function that manages the UDP responses from the host and updates the LED's accordingly

 */
int hostReceiver::readThread()
{

	while(this->isRunning)
	{
		//Read the data in from the UDP interface

		if (server.receiveUdp(reinterpret_cast<char*>(this->rcvbuf), sizeof(this->rcvbuf)) > 0)
		{
			this->onMessageReceived();

			//Trigger to start recording
			if(this->commandedState == RECORDING && this->hostState == STANDBY)
			{
				std::cout<<"Start recording\n";
				this->startRecording();
			}
			else if (this->commandedState == STANDBY && this->hostState == RECORDING)
			{
				std::cout<<"Stop recording\n";
				this->stopRecording();
			}
		}
		//Run at 10Hz
		usleep(100000);
	}
	return 0;
}

/** Function that manages the UDP commands to the host

 */
int hostReceiver::writeThread()
{
	//thread whih keeps the heartbeat sending
	while(this->isRunning)
	{
		this->createSendMessage();
		this->server.send(reinterpret_cast<char*>(this->sndbuf), sizeof(this->sndMessage));

		//Send a command message at 10Hz
		usleep(100000);
	}
	return 0;
}

/** Fill Message to send

 */
int hostReceiver::createSendMessage()
{
	//Manually fill out the contents of the message to sent
	this->sndMessage.magicHeader1 = MAGIC_H1;
	this->sndMessage.magicHeader2 = MAGIC_H2;
	this->sndMessage.isCommandMsg = 0u;
	this->sndMessage.isStatusMsg = 1u;
	if (this->hostState == RECORDING)
		this->sndMessage.mode = MODE_RECORDING;
	else if (this->hostState == STANDBY)
		this->sndMessage.mode = MODE_STANDBY;
	this->sndMessage.magicFooter1 = MAGIC_F1;
	this->sndMessage.magicFooter2 = MAGIC_F2;

	//Copy the message into our buffer for sending
	memcpy(this->sndbuf, &this->sndMessage, sizeof(this->sndMessage));
	return 0;
}


//TODO, set the udpserver such that this function is called every time a UDP message is received
int hostReceiver::onMessageReceived()
{
	//Copy the buffer into the messge predefined message
	memcpy(&this->rcvMessage, this->rcvbuf, sizeof(this->rcvMessage));

	//Check the magic numbers
	if( 
		this->rcvMessage.magicHeader1 == MAGIC_H1
		&& this->rcvMessage.magicHeader2 == MAGIC_H2
		&& this->rcvMessage.magicFooter1 == MAGIC_F1
		&& this->rcvMessage.magicFooter2 == MAGIC_F2
		)
	{
		//Update our local variables with the contents of the new message
		this->lastTimestampReceived_us = this->rcvMessage.timestamp_us;
		this->lastModeReceived = this->rcvMessage.mode;
	
		if(this->lastModeReceived == MODE_RECORDING)
		{
			this->commandedState = RECORDING;
		}
		else if(this->lastModeReceived == MODE_STANDBY)
		{
			this->commandedState = STANDBY;
		}
	}
	return 0;
}

int hostReceiver::startRecording()
{
	system("/home/msardonini/Videos/record_on_boot.sh &");
	this->hostState = RECORDING;
}


int hostReceiver::stopRecording()
{
	system("/home/msardonini/Videos/stopProgram.sh &");
	this->hostState = STANDBY;
}



uint64_t hostReceiver::getTimeUsec()
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return tv.tv_sec*(uint64_t)1E6 + tv.tv_nsec/(uint64_t)1E3;
}
