/**
 * @file remoteSender.cpp
 * @brief Application entry point for the remote sender
 *
 * @author Mike Sardonini
 * @date 09/10/2018
 */

//System
#include<iostream>
#include<vector>
#include<memory>
#include<unistd.h>

//Packages

//Ours
#ifdef REMOTE_SENDER
    #include "remoteSender.h"
#elif HOST_RECEIVER
    #include "hostReceiver.h"
#endif

//local functions
static void print_usage();


//Application Entry Point
int main(int argc, char *argv[])
{
    int c;
    char* hostIP = NULL;
    bool useBluetooth = false;
    while ((c = getopt (argc, argv, "i:hb")) != -1)
    {
        switch (c)
        {
             //Get the IP address to use
            case 'i':
                hostIP = static_cast<char*>(calloc(512, sizeof(char)));
                if(sscanf(optarg, "%15[^,]", hostIP) <= 0)
                {
                    std::cerr << "Failed to read IP address" << std::endl;
                    return 0;
                }
                break;

            //Print out the help guide
            case 'h':
                print_usage();
                return 0;

            case 'b':
                useBluetooth = true;
                break;
            //Handle unknown Arguments
            case '?':
                if (optopt == 'c')
                  std::cerr << "Option -%c requires an argument.\n" << optopt;
                else if (isprint (optopt))
                  std::cerr << "Unknown option `-%c'.\n" << optopt;
                else
                    std::cerr << "Unknown option character " << optopt;
                    print_usage();
                return 1;
            
            //Error on Unknown input Argument
            default:
                std::cout <<"Error! Unknown argument given";
                print_usage();
                return 0;
        }
    }

    if (hostIP == NULL)
    {
        hostIP = static_cast<char*>(calloc(512, sizeof(char)));
        strcpy(hostIP, "127.0.0.1");
    }

#ifdef REMOTE_SENDER

    remoteSender* receiver;
    if(useBluetooth)
        receiver = new remoteSender;
    else
    {
        std::string remoteIPstring(hostIP);
        receiver = new remoteSender(remoteIPstring);
    }
#elif HOST_RECEIVER
    hostReceiver* receiver;
    if(useBluetooth)
    {   
        receiver = new hostReceiver;

        while(1)
        {
            //Check to see if the class needs to be reset
            if(receiver->getNeedsReset())
            {
                delete receiver;
                sleep(4);

                receiver = new hostReceiver;
            }

            sleep(1);
        }
        // receiver = std::make_unique<hostReceiver>();
    }
    else
    {
        std::string remoteIPstring(hostIP);
        receiver = new hostReceiver(remoteIPstring);
    }
#endif

	//Just sleep in main while the remoteSender operates
	while(1)
	{
		sleep(1);
	}


	return 0;
}

static void print_usage(){
    std::cout <<"\n Usage:\n";
    std::cout <<"./remoteSender [-OPTION OPTION_VALUE]\n";
    std::cout <<"\n";
    std::cout <<"Options:\n";
    std::cout <<"-i {ip Address} 		Manually set the IP address of the host to connect to. Default: 127.0.0.1 (localhost)\n";
    std::cout <<"-h {help}                  Print this usage text\n";
    
    return;
}

