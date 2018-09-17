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
	hostState(DISCONNECTED),
	buttonState(DISCONNECTED),
	isRunning(true)
{
	//Port to read from the headless machine
	int portRemote = 200;
	int portHost = 201;

	this->server = UdpServer();

	//Connect to Any available local IP address and listen on given port 
	this->server.connect("", portRemote, 256);
	this->server.setClientInfo(ipAddr, portHost);

	//Start the thread that handles our LEDs
	this->redLedThread = std::thread(&remoteSender::LedControlThread, this, RED);
	this->greenLedThread = std::thread(&remoteSender::LedControlThread, this, GREEN);

	this->readThread_h = std::thread(&remoteSender::readThread, this);
	this->writeThread_h = std::thread(&remoteSender::writeThread, this);
	
	//Button Thread
	this->buttonThread_h = std::thread(&remoteSender::buttonThread, this);

	//Initialize the GPIO pints for LEDs and Butons
	wiringPiSetup();

	//Inintialize the GREEN LED
	pinMode(GPIO_GREEN_LED, OUTPUT);
	pinMode(GPIO_GREEN_BUTTON, INPUT);
  
	//Inintialize the GREEN LED
	pinMode(GPIO_RED_LED, OUTPUT);
	pinMode(GPIO_RED_BUTTON, INPUT);
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

	uint64_t previousTimeStamp_us;
	while(this->isRunning)
	{
		//Read the data in from the UDP interface
		if(server.receiveUdp(reinterpret_cast<char*>(this->rcvbuf), sizeof(this->rcvbuf)) > 0)
		{
			previousTimeStamp_us = this->getTimeUsec();
			this->onMessageReceived();
		}

		//Check if we have received the heartbeat status message in a reasonable amount of time
		if (abs(this->getTimeUsec() - previousTimeStamp_us) > 1e6)
		{
			this->hostState = DISCONNECTED;
		}
		else if (this->hostState == DISCONNECTED)
		{
			this->hostState = STANDBY;
		}
		
		// Update the LED status
		switch(this->hostState)
		{
			case DISCONNECTED:
				this->setLedFlashing(GREEN);
				this->setLedOff(RED);
				break;
			case STANDBY:
				this->setLedOn(GREEN);
				this->setLedOff(RED);
				break;
			case RECORDING:
				this->setLedOff(GREEN);
				this->setLedOn(RED);
				break;
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
	this->sndMessage.isCommandMsg = 1u;
	this->sndMessage.isStatusMsg = 0u;
	if (this->buttonState == RECORDING)
		this->sndMessage.mode = MODE_RECORDING;
	else
		this->sndMessage.mode = MODE_STANDBY;
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
			this->hostState = RECORDING;
		}
		else if(this->lastModeReceived == MODE_STANDBY)
		{
			this->hostState = STANDBY;
		}
	}
	return 0;
}

int remoteSender::buttonThread()
{

	int redButtonState = 0;
	int previousRedButtonState = 0;


	int greenButtonState = 0;
	int previousGreenButtonState = 0; 

	while(this->isRunning)
	{
		previousGreenButtonState = greenButtonState; 
		greenButtonState = digitalRead(GPIO_GREEN_BUTTON);

		// Detect a green button push
		if (greenButtonState > previousGreenButtonState)
		{
			// std::cout << "green push detected \n";
			this->buttonState = STANDBY;
		}

		previousRedButtonState = redButtonState; 
		redButtonState = digitalRead(GPIO_RED_BUTTON);
		
		// Detect a red button push
		if (redButtonState > previousRedButtonState)
		{
			// std::cout << "red push detected \n";
			this->buttonState = RECORDING;
		}

		//Run at 100hz to catch quick button pushes
		usleep(10000);
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

			case FLASHING:
				isLedOn = (isLedOn) ? false : true;
				if(isLedOn)
				{
					if (color == RED)
						digitalWrite(GPIO_RED_LED, 0);
					else if(color == GREEN)
						digitalWrite(GPIO_GREEN_LED, 0);

				}
				else
				{
					if (color == RED)
						digitalWrite(GPIO_RED_LED, 1);
					else if(color == GREEN)
						digitalWrite(GPIO_GREEN_LED, 1);					
				}
				break;
		}
		usleep(500000);
	}
	return 0;
}

//Sets the LED to flashing
int remoteSender::setLedFlashing(enum LED_COLORS_t color)
{
	if (color == RED)
		this->redLedStatus = FLASHING;
	else if (color == GREEN)
		this->greenLedStatus = FLASHING;

	return 0;
}

//Turns on the LED
int remoteSender::setLedOn(enum LED_COLORS_t color)
{
	if (color == RED)
		this->redLedStatus = ON;
	else if (color == GREEN)
		this->greenLedStatus = ON;

	return 0;
}

//Turns off the LED
int remoteSender::setLedOff(enum LED_COLORS_t color)
{
	if (color == RED)
		this->redLedStatus = OFF;
	else if (color == GREEN)
		this->greenLedStatus = OFF;

	return 0;
}


uint64_t remoteSender::getTimeUsec()
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return tv.tv_sec*(uint64_t)1E6 + tv.tv_nsec/(uint64_t)1E3;
}
