/*
 * server.h
 *
 *  Created on: Feb, 2016
 *      Author: Manisha Agrawal
 */

#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define SLEEP_TIME   	20000		//Sleep to synchronize data transfer
#define PORT 			5000		//Listening port
#define SERVICE_FILE 	"services.txt"//Store client details here
#define SIZE 			1024		//Size of the buffer
#define CHUNK_SIZE  	65			//Chunksize+1
#define MAX_CLIENTS	    5			//Maximum number of clients

/****************Function declarations**********************/
int find_service(char *buf, char *dis_buf);	//Find a service for a client
void register_service(char *buf);			//Register a new client to the directory
