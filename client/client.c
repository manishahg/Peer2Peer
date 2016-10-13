/*
 * client.c
 *
 *  Created on: Feb, 2016
 *      Author: Manisha Agrawal
 */
#include "client.h"

fd_set master, read_fds;

char FILE_VECTOR[CHUNK_SIZE] = "000000000000000000000000000000000000000000000000000000000000000";
char SERVERNODE[100], SERVERIP[40];
char ConfigFile[100], config_msg[SIZE];
int serversockfd = -1, highestsocket;
int MYPEERID, SERVERPORT, MY_LISTEN_PORT;

//Connect to server
int connect_to_server()
{
    struct sockaddr_in server_addr; // peer address\n";
    int sockfd;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Cannot create a socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;    // host byte order
    server_addr.sin_port = htons(SERVERPORT);  // short, network byte order
    inet_aton(SERVERIP, (struct in_addr *)&server_addr.sin_addr.s_addr);
    memset(&(server_addr.sin_zero), '\0', 8);  // zero the rest of the struct

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
         printf("Error connecting to the server %s on port %d\n",SERVERIP,SERVERPORT);
     	 sockfd = -1;
    }

    struct sockaddr_in my_addr;
    int len = 16;
    getsockname(sockfd, (struct sockaddr *)&my_addr,  (socklen_t *)&len);//sizeof(struct sockaddr_in));

    if (sockfd != -1)
    {
 	    FD_SET(sockfd, &master);
 	    if (highestsocket <= sockfd)
	    {
            highestsocket = sockfd;
	    }
    }

	return sockfd;
}

//Read the input from user
void process_stdin_message()
{
	char buf[SIZE], message[SIZE];
	char choice[SIZE];
	int chunk, rcv_count;

	fgets(choice, SIZE, stdin);
	chunk = atoi(choice);

	//Choice 'f' is to request a chunk number
	if(choice[0]=='f')
    {
		printf("Please enter chunk number<1-64>\n");
		return;
    }
	//chunk number should be between 1 and 64
	else if((chunk>1) && (chunk<64))
	{
		//Check if requested chunk already exists
		if(FILE_VECTOR[chunk-1]=='1')
		   printf("I already have chunk %d\n",chunk);
		//else ask send server the required chunk number
		else
		{
			strcpy(message,"Discover");
			if (send(serversockfd , message , strlen(message) , 0) < 0)
			{
				   perror("Could not connect to the Server!!\n");
				   exit(1);
			}

			char CHUNK_VECTOR[10] ;
			sprintf(CHUNK_VECTOR, "%d", chunk);
			if (send(serversockfd , CHUNK_VECTOR , strlen(CHUNK_VECTOR) , 0) < 0)
			{
				   perror("Could not connect to the Server!!\n");
				   exit(1);
			}

			//Discovered successfully
			buf[0]='\0';
			rcv_count= recv(serversockfd , buf , SIZE , 0) ;
			if((rcv_count<=0) || (strstr(buf,"Exit")!=NULL))
       	    {
                printf("Chunk:Server hung up. I will also quit.\n");
				exit(0);
            }
			else if(strstr(buf,"Failed")!=NULL)
				printf("Chunk not available!!\n");
			else
				printf("Chunk %d avaialable at:\n%s\n",chunk,buf);
		}
    }
	//Enter 'q' to quit the client
	else if(choice[0]=='q')
	{
		strcpy(message, "Quit");

		if (send(serversockfd , message , strlen(message) , 0) < 0)
		{
		   perror("Could not connect to the Server!!\n");
		   exit(1);
		}
		buf[0]='\0';
		if( recv(serversockfd , buf , SIZE , 0) < 0)
			return;

		//Close the connection when server sends exit
		if(strstr(buf,"Exit")!=NULL)
		{
		  close(serversockfd);
		  exit(0);
		}
	}
	return;
}

//Read incoming data from sockets
void read_from_activesockets(void)
{
    char buf[SIZE];    // buffer for client data
    int nbytes;

    if (FD_ISSET(fileno(stdin),&read_fds))
    	process_stdin_message();
    else if ((serversockfd != -1) && FD_ISSET(serversockfd,&read_fds) )
    {
	    nbytes = recv(serversockfd, buf, SIZE, 0);
        // handle server response or data
    	if ( nbytes <= 0)
  	    {
            // got error or connection closed by client
            if (nbytes == 0)
       	    {
                printf("Server hung up. I will also quit.\n");
				exit(0);
            }
	        else
	        {
	            printf("client %d recv error from server \n", MYPEERID);
            }
            close(serversockfd); // bye!
            FD_CLR(serversockfd, &master); // remove from master set
            serversockfd = -1;
        }
    }
}

//Read client details from configuration file
void read_config()
{
    FILE* f = fopen(ConfigFile,"r");

    if (f)
    {
		fscanf(f,"CLIENTID %d\n",&MYPEERID);
	    fscanf(f,"SERVERPORT %d\n",&SERVERPORT);
	    fscanf(f,"MYPORT %d\n",&MY_LISTEN_PORT);
	    fscanf(f,"FILE_VECTOR %s\n",FILE_VECTOR);

		fclose(f);

        printf("My ID is %d\n", MYPEERID);
        printf("Sever port is %d\n", SERVERPORT);
        printf("My port is %d\n", MY_LISTEN_PORT);
        printf("File vector is <%s>\n", FILE_VECTOR);

        config_msg[0]='\0';
        sprintf(config_msg, "Chunk:%s\nID:%d\nPort:%d\n",FILE_VECTOR,MYPEERID,MY_LISTEN_PORT);
    }
    else
    {
        printf("Cannot read the config file! :%s\n", ConfigFile);
     	fflush(stdout);
        exit(1);
    }
}

//Execute client
void client_run()
{
	// clear the master and temp sets
	FD_ZERO(&master);
    FD_ZERO(&read_fds);
    highestsocket =0;

    //Connect with the server
    serversockfd =  connect_to_server();
    if(serversockfd<=0)
    {
    	printf("Error: Could not connect with the server\n");
    	exit(1);
    }

    //Register the client with the server
    int send_status= send(serversockfd, "Register", 10, 0);
    if(send_status<=0)
    {
    	printf("Error: Could not connect with the server\n");
    	exit(1);
    }

    usleep(SLEEP_TIME);
    send_status= send(serversockfd, config_msg, strlen(config_msg), 0);
    if(send_status<=0)
    {
    	printf("Error: Could not connect with the server\n");
    	exit(1);
    }

     FD_SET(fileno(stdin), &master);
     if (fileno(stdin) > highestsocket)
     {
      	highestsocket = fileno(stdin);
     }

    // main loop
    while (1)
    {
	    struct timeval timeout;
	    timeout.tv_sec = 1;
	    timeout.tv_usec = 100;

        read_fds = master;

        if (select(highestsocket+1, &read_fds, NULL, NULL, &timeout) == -1)
        {
            if (errno == EINTR)
    	    {
                printf("Select for client %d interrupted by interrupt...\n", MYPEERID);
    	    }
    	    else
    	    {
                printf("Select problem .. client %d exiting iteration\n", MYPEERID);
		        fflush(stdout);
                exit(1);
            }
        }

        read_from_activesockets();
    }
}


int main(int argc, char** argv)
{
	struct hostent *server123;

	if(argc!=3)
    {
    	printf( "Usage: ./client ServerIP config_filename\n");
    	exit(1);
    }

    strcpy(ConfigFile,argv[2]);
    read_config();

    ////Find out the server IP address
    if ((server123=gethostbyname(argv[1])) != NULL)
    	strcpy(SERVERIP,inet_ntoa(*(struct in_addr *)server123->h_addr_list[0]));

	printf("Please enter your choices\n'f'-To request for a chunk.\n'q'-To quit.\n");
    client_run();

    return 0;
}

