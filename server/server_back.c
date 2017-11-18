#include "ftpserver.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

unsigned short int ptnum;
int server_socket;
char* addr;

void createconn(int port,char* address)
{
	pthread_t listen_thread;
	printf("ftpserver\n");
	if(address == NULL) printf("The server port is %d.\n\n",port);
	else printf("The server address is %s:%d.\n\n",address,port);
	ptnum = htons((unsigned short int)port);
	addr = address;
	if(pthread_create(&listen_thread,0,startlisten,0)!=0)
	{
		printf("Error while creating thread.\n");
		exit(1);
	}
}

void* startlisten(void* NOT_TO_USE)
{
	struct sockaddr_in sock;
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = (addr == 0 ? htonl(INADDR_ANY) : inet_addr(addr));
	//如果没有给出地址，就用自动绑定的地址
	sock.sin_port = ptnum;
	server_socket = socket(AF_INET,SOCK_STREAM,0);
	if(bind(server_socket,(const struct sockaddr*)&sock,sizeof(sock))!=0)
	{
		printf("Error while binding.\n");
		exit(1);
	}
	if(listen(server_socket,CONNECTION)!=0)
	{
		printf("Error while listening.\n");
		exit(1);
	}
	while(1)
	{
		int client_socket = accept(server_socket,(struct sockaddr *)0,0);
		if(count_curr(false) == CONNECTION)
		{
			char retValue = -1;
			write(client_socket,&retValue,sizeof(char));
		}
		else
		{
			pthread_t session_thread;
			if(pthread_create(&session_thread,0,user_session_main,(void*)client_socket)!=0)
			{
				char retValue = -2;
				write(client_socket,&retValue,sizeof(char));
				return (void*)0;
			}
			else
			{
				//注意：如果线程创建成功应该是向客户返回0的，但由于线程已经创建成功，
				//因此向客户返回0的工作由新线程来执行，故在此不执行返回值的代码。
			}
		}

	}
	return (void*)0;
}
