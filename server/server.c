/*
 * server.c
 *
 *  Created on: Feb, 2016
 *      Author: Manisha Agrawal
 */

#include "server.h"

int main(int argc , char *argv[])
{
    //set of socket descriptors
    fd_set readfds;
    struct sockaddr_in address;

    int master_socket , addrlen , new_socket ;
    int opt=1, client_socket[MAX_CLIENTS] ;
    int activity, i , valread , sd, max_sd;
    char buffer[SIZE], buf[SIZE], dis_buf[SIZE];  //data buffer of 1K
    char quit_buffer[SIZE], my_buffer[SIZE];

    //initialise all client_socket[] to 0
    for (i = 0; i < MAX_CLIENTS; i++)
        client_socket[i] = 0;

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to port
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
	printf("Listener on port %d \n", PORT);

    //try to specify maximum of MAX_CLIENTS pending connections for the master socket
    if (listen(master_socket, MAX_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    printf("Waiting for connections ...\n");

	while(1)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        FD_SET(0, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < MAX_CLIENTS ; i++)
        {
            //socket descriptor
			sd = client_socket[i];

			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
				max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR))
	    {
			printf("select error\n");
			exit(EXIT_FAILURE);
	    }

        //Quit the connection if user inputs 'q'
        if (FD_ISSET(0, &readfds))
        {
        	fgets(quit_buffer, SIZE, stdin);

        	//Before quitting, close all the connections with client
        	if(!strcmp(quit_buffer, "q\n"))
            {
				strcpy(buf, "Exit");
		        for (i = 0; i < MAX_CLIENTS; i++)
		        {
		        	if( client_socket[i] == 0 )
		        		continue;
		        	sd = client_socket[i];
					send(sd , buf , strlen(buf) , 0) ;

					//Close the socket and mark as 0 in list for reuse
					close( sd );
					client_socket[i] = 0;
		        }
		        exit(0);
            }
        }

        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            //add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                //if position is empty
				if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
					break;
                }
            }
        }

        //else its some IO operation on some other socket :)
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds))
            {
                //Read the incoming message
            	memset(buffer, '\0', sizeof(buffer));

            	valread = read( sd , buffer, SIZE);
                if ( valread == 0)
                {
                    //Client disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }

                //process the message that came in
                else
                {
                	buffer[valread]='\0';
                	my_buffer[0]='\0';
                	strcpy(my_buffer,buffer);

					//Check if clients requested to Quit
					if((!strcmp(my_buffer,"Quit")) || (!strcmp(my_buffer,"q\n")))
					{
						usleep(SLEEP_TIME); // wait a while
						strcpy(buf, "Exit");
						send(sd , buf , strlen(buf) , 0) ;

						//Somebody disconnected , get his details and print
						getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
						printf("Host Quit : ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

						//Close the socket and mark as 0 in list for reuse
						close( sd );
						client_socket[i] = 0;

					}

					//Register the client
					else if(!strcmp(my_buffer,"Register"))
					{
						buf[0]='\0';
						//receive service information
						if(recv(sd , buf , SIZE , 0)<=0)
						{
							printf("register recv failed\n");
							continue;	//Add new client to the directory
						}

						printf("<%s>\n",buf);
						register_service(buf);
						usleep(SLEEP_TIME); // wait a while
					}
					//Look for the requested chunk
					else if(!strcmp(my_buffer, "Discover"))
					{
						usleep(SLEEP_TIME); // wait a while
						buf[0]='\0';

						//Print the details of client who requested the service
						getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
						printf("Host : ip %s , port %d\n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

						//receive chunk request information
						dis_buf[0]='\0';
						if(recv(sd , buf , SIZE , 0)>0)
						{
							//find the requested service in directory
							if(find_service(buf,dis_buf)!=0)
								strcpy(dis_buf, "Failed");

						}
						else
							strcpy(dis_buf, "Failed");

						//Send Service status
						send(sd , dis_buf , strlen(dis_buf) , 0) ;
					}
					else
					{
						//Service not available
						strcpy(dis_buf, "Failed");
						send(sd , dis_buf , strlen(dis_buf) , 0) ;
					}
                }
            }
        }
    }

    return 0;
}
