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
	: greenLedStatus(FLASHING),
	redLedStatus(OFF),
	remoteState(DISCONNECTED),
	hostState(DISCONNECTED),
	isRunning(true)
{
	//Port to read from the headless machine
	int portRemote = 200;
	int portHost = 201;

	this->server = UdpServer();

	//Connect to Any available local IP address and listen on given port 
	this->server.connect("", portHost, 256);
	this->server.setClientInfo(ipAddr, portRemote);

	this->readThread_h = std::thread(&remoteSender::readThread, this);
	this->writeThread_h = std::thread(&remoteSender::writeThread, this);
	
}

/** Default Destructor

 */
remoteSender::~remoteSender()
{


}

/** Function that manages the UDP responses from the host and updates the LED's accordingly

 */
int remoteSender::readThread()
{

	while(this->isRunning)
	{
		//Read the data in from the UDP interface
		server.receiveUdp(reinterpret_cast<char*>(this->rcvbuf), sizeof(this->rcvbuf));
		
		this->onMessageReceived();

		//Check if we have received the heartbeat status message in a reasonable amount of time
		if (this->getTimeUsec() - this->lastTimestampReceived_us > 1e6)
		{
			this->remoteState = DISCONNECTED;
		}
		else if (this->remoteState == DISCONNECTED)
		{
			this->remoteState = STANDBY;
		}

		//Run at 10Hz
		usleep(100000);
	}
	return 0;
}

/** Function that manages the UDP commands to the host

 */
int remoteSender::writeThread()
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
int remoteSender::createSendMessage()
{
	//Manually fill out the contents of the message to sent
	this->sndMessage.magicHeader1 = MAGIC_H1;
	this->sndMessage.magicHeader2 = MAGIC_H2;
	this->sndMessage.isCommandMsg = 0u;
	this->sndMessage.isStatusMsg = 1u;
	if (this->hostState == RECORDING)
		this->sndMessage.mode = 1u;
	else
		this->sndMessage.mode = 0u;
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
			this->remoteState = RECORDING;
		}
		else if(this->lastModeReceived == MODE_STANDBY)
		{
			this->remoteState = STANDBY;
		}
	}
	return 0;
}


int remoteSender::LedControlThread(enum LED_COLORS_t color)
{
	enum LED_STATUS_t status;
	bool isLedOn = false;

	while(this->isRunning)
	{
		if (color == RED)
			status = this->redLedStatus;
		else if (color == GREEN)
			status = this->greenLedStatus;

		switch (status)
		{
			case ON:
				if (color == RED)
					digitalWrite(GPIO_RED_LED, 1);
				else if(color == GREEN)
					digitalWrite(GPIO_GREEN_LED, 1);
				isLedOn = true;
				break;

			case OFF:
				if (color == RED)
					digitalWrite(GPIO_RED_LED, 0);
				else if(color == GREEN)
					digitalWrite(GPIO_GREEN_LED, 0);
				isLedOn = false;
				break;
		}
		usleep(500000);
	}
	return 0;
}



uint64_t remoteSender::getTimeUsec()
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return tv.tv_sec*(uint64_t)1E6 + tv.tv_nsec/(uint64_t)1E3;
}
