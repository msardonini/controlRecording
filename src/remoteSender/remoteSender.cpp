/**
 * @file remoteSender.cpp
 * @brief Control the status of a video recorder on a headless machine using a remote raspberry pi
 *
 * @author Mike Sardonini
 * @date 09/10/2018
 */

#include "remoteSender.h" 



/** Default Constructor

 */
remoteSender::remoteSender(std::string ipAddr)
{
	//Port to read from the headless machine
	int port = 200;

	this->server = UdpServer();

	//Connect to Any available local IP address and listen on given port 
	this->server.connect("", port, 256);

	this->server.setClientInfo(ipAddr, port);

}

/** Default Destructor

 */
remoteSender::~remoteSender()
{


}


/** function to run the program

 */
int remoteSender::run()
{


	//thread whih keeps the heartbeat sending
	while(this->isRunning)
	{
		//Check if we have received the heartbeat status message in a reasonable amount of time
		if (this->timestamp_us - this->lastTimestampReceived_us > 1e6)
		{
			this->setLedFlashing();
		}
		else
		{
			this->setLedSolid();
		}

		this->createSendMessage();
		this->server.send(reinterpret_cast<char*>(this->sndbuf), sizeof(this->sndMessage));

		usleep(100000);
	}

}

/** Fill Message to send

 */
int remoteSender::createSendMessage()
{
	//Manually fill out the contents of the message to sent
	this->sndMessage.magicHeader1 = MAGIC_H1;
	this->sndMessage.magicHeader2 = MAGIC_H2;
	this->sndMessage.isCommandMsg = 1u;
	this->sndMessage.isStatusMsg = 0u;
	this->sndMessage.mode = this->mode;
	this->sndMessage.magicFooter1 = MAGIC_F1;
	this->sndMessage.magicFooter2 = MAGIC_F2;

	//Copy the message into our buffer for sending
	memcpy(this->sndbuf, &this->sndMessage, sizeof(this->sndMessage));
	return 0;
}


//TODO, set the udpserver such that this function is called every time a UDP message is received
int remoteSender::onMessageReceived()
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
			this->setLedRecording();
		}
		else if(this->lastModeReceived == MODE_STANDBY)
		{
			this->setLedRecordingOff();
		}
	}
	return 0;
}

//TODO Make the fuctions interface with actual LEDS on a pi

int remoteSender::setLedFlashing()
{
	return 0;
}

int remoteSender::setLedSolid()
{
	return 0;
}

int remoteSender::setLedRecording()
{
	return 0;
}

int remoteSender::setLedRecordingOff()
{
	return 0;
}