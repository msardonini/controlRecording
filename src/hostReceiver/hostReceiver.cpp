/**
 * @file hostReceiver.cpp
 * @brief Control the status of a video recorder on a headless machine using a remote raspberry pi
 *
 * @author Mike Sardonini
 * @date 09/16/2018
 */

#include "hostReceiver.h" 



/** Bluetooth interface Constructor

 */
hostReceiver::hostReceiver()
	: commandedState(STANDBY),
	hostState(STANDBY),
	isRunning(true),
	useBluetooth(true),
	useUDP(false)
{
	printf("Bluetooth!\n"); 
	struct termios  config;

	const char *device = "/dev/rfcomm0";

   struct stat buf;
    while (stat(device, &buf))
    	sleep(1);


	this->fd = open(device, (O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK));
	if(this->fd == -1) {
		printf( "failed to open port\n" );
		return;
	}

	//Check if the serial port is a tty device
	if(!isatty(this->fd))
	{ 
		printf("Error, invalid serial device\n"); 
	}

	if(tcgetattr(this->fd, &config) < 0) 
	{ 
		printf("Error reading config from serial device\n"); 
	}

	config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |INLCR | PARMRK | INPCK | ISTRIP | IXON);
	config.c_oflag = 0;
	config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	config.c_cflag &= ~(CSIZE | PARENB);
	config.c_cflag |= CS8;
	config.c_cc[VMIN]  = 10; //Miniumum size of 10 bytes to return from read
	config.c_cc[VTIME] = 100; //return from read after 100 microseconds

	//Set the read and write speeds
	if(cfsetispeed(&config, B115200) < 0 || cfsetospeed(&config, B115200) < 0) 
	{
		printf("Error setting termios speed\n");
	}

	//Apply the settings we have made
	if(tcsetattr(this->fd, TCSAFLUSH, &config) < 0) 
	{
		printf("Error setting termios attributes\n");
	}
	this->readThread_h = std::thread(&hostReceiver::readThread, this);
	this->writeThread_h = std::thread(&hostReceiver::writeThread, this);	
}

/** UDP communication interface Constructor

 */
hostReceiver::hostReceiver(std::string ipAddr)
	: commandedState(STANDBY),
	hostState(STANDBY),
	isRunning(true),
	useBluetooth(false),
	useUDP(true)
{
	printf("Network!\n"); 
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
	this->isRunning = false;
	if(this->readThread_h.joinable())
		this->readThread_h.join();
	if(this->writeThread_h.joinable())
		this->writeThread_h.join();

}

/** Function that reads data from the other device

 */
ssize_t hostReceiver::receiveData()
{
	if (this->useBluetooth)
	{
		return read(this->fd, reinterpret_cast<char*>(this->rcvbuf), sizeof(this->rcvbuf));
	}
	else if(this->useUDP)
	{
		return server.receiveUdp(reinterpret_cast<char*>(this->rcvbuf), sizeof(this->rcvbuf));
	}
}

/** Function that manages the UDP responses from the host and updates the LED's accordingly

 */
int hostReceiver::readThread()
{

	while(this->isRunning)
	{
		//Read the data in from the comminication interface

		ssize_t ret = this->receiveData();
		if (ret)
		{
			this->onMessageReceived();

			//Trigger to start recording
			if(this->commandedState == RECORDING && this->hostState == STANDBY)
			{
				this->startRecording();
			}
			else if (this->commandedState == STANDBY && this->hostState == RECORDING)
			{
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

		if(this->useBluetooth)
			write(this->fd, reinterpret_cast<char*>(this->sndbuf), sizeof(this->sndMessage));
		else if(this->useUDP)
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
