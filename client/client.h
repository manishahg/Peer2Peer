/*
 * client.h
 *
 *  Created on: Feb, 2016
 *      Author: Manisha Agrawal
 */

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <errno.h>

#define SLEEP_TIME   20000	//Sleep to synchronize data transfer
#define CHUNK_SIZE   65		//Chunksize+1
#define SIZE		 1024	//Size of buffer

/****************Function declarations**********************/
int get_config(char *, int *, char *, char *);	//Get details from config file
void read_from_activesockets(void);				//Read incoming data from sockets
void read_config(); 							//Read client details from configuration file
int connect_to_server();						//Connect to server
void process_stdin_message();					//Read the input from user
void client_run();								//Execute client

