#include "myftp_header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int createconn(char* address,int port)
{
	struct sockaddr_in sock;
	sock.sin_family=AF_INET;
	sock.sin_addr.s_addr=inet_addr(address);
	sock.sin_port=htons(port);
	int client_socket=socket(AF_INET,SOCK_STREAM,0);
	if(connect(client_socket,(struct sockaddr *)&sock,sizeof(sock))!=0)
	{
		fprintf(stderr,"Error while connecting.\n");
		exit(1);
	}
	else
	{
		char retValue;
		read(client_socket,&retValue,sizeof(char));
		if(retValue == -1)
		{
			fprintf(stderr,"The server has reached its maximum capacity, please try again later.\n");
			exit(1);
		}
		else if(retValue == -2)
		{
			fprintf(stderr,"The server has been crashed. Contact the server manager to know more.\n");
			exit(1);
		}
		else
		{

		}
	}
	return client_socket;
}

int identify(int socket,char* id_str)
{
	//返回正数表示成功（第几位用户），-1表示失败。
	write(socket,id_str,strlen(id_str)+1);
	char retValue;
	read(socket,&retValue,sizeof(char));
	return (int)retValue;
}
