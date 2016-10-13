/*
 * functions.c
 *
 *  Created on: Feb, 2016
 *      Author: Manisha Agrawal
 */
#include "server.h"

//Register a new client to the server
void register_service(char *buf)
{
	FILE *fp;
	char msg[SIZE], id_details[SIZE];
	fp = fopen(SERVICE_FILE,"r+"); // read mode

	//If file does not exist create it
	if( fp == NULL )
	{
		fp=fopen(SERVICE_FILE,"w+");
		fputs(buf, fp);
		fputs("\n\n", fp);
		fclose(fp);
		return ;
	}

	id_details[0]='\0';
	char *clientid = strstr(buf,"ID");
	if(clientid != NULL)
	{
		strcat(id_details,"ID:");
		int i=0;
		while(*clientid!='\n')
		{
			id_details[i++]=*clientid;
			clientid++;
		}
		id_details[i]='\0';
	}

	//Check if the client is already registered
	while(fgets(msg, SIZE, fp) != NULL)
	{
		if(strstr(msg,id_details)!=NULL)
			return;
	}

	fputs(buf, fp);
	fclose(fp);
	return ;
}


//Find the chunk owner details for a client
int find_service(char *buf, char *dis_buf)
{
	FILE *fp;
	char msg[SIZE], info[SIZE], new_vector[SIZE];
	int chunk;

	fp = fopen(SERVICE_FILE,"r"); // read mode

	//Check if file  exist
	if( fp == NULL )
		return 1;

	chunk=atoi(buf);
	printf("Requested chunk:%d\n",chunk);

	//look for requested chunk number
	while(fgets(msg, SIZE, fp) != NULL)
	{
		if(strstr(msg, "Chunk") == NULL)
			continue;
		else
		{

			char *chunkvector = strchr(msg, ':');
			if (chunkvector != NULL)
			{
				//new_vector[i++]=*chunkvector;
				chunkvector++;
			}
			strcpy(new_vector, chunkvector);

			if(new_vector[chunk-1]=='1')
			{
				//Send portno. and client id
				info[0]='\0';
				fgets(msg, SIZE, fp); //get client id
				strcat(info, msg);
				fgets(msg, SIZE, fp); //get client's port number
				strcat(info, msg);
				strcpy(dis_buf,info);
				fclose(fp);
				printf("<Chunk %d avaialable at:\n%s>\n",chunk,dis_buf);
				return 0;
			}
		}
	}

	fclose(fp);
	return 1;
}
