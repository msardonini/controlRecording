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
    while ((c = getopt (argc, argv, "i:h")) != -1)
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
    std::string hostIPstring;
    hostIPstring.append(hostIP);
	remoteSender sender(hostIPstring);
#elif HOST_RECEIVER
    std::string remoteIPstring;
    remoteIPstring.append(hostIP);
    hostReceiver receiver(remoteIPstring);
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

