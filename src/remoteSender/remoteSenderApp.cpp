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
#include "remoteSender.h"


int main(int argc, char *argv[])
{


	remoteSender sender();

	//Just sleep in main while the remoteSender operates
	while(1)
	{
		sleep(1);
	}


	return 0;
}