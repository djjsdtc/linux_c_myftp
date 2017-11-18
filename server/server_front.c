#include "ftpserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <grp.h>

extern int server_socket;		//定义在server_back.c内
extern useritem items[CONNECTION];	//定义在server_back.c内

int main(int argc, char* argv[])
{
	if(getuid() != 0)
	{
		printf("You must run this program under the root privilege.\n");
		exit(0);
	}
	if(getgrnam("ftp") == NULL)
	{
		printf("Usergroup \"ftp\" not found in your system.\n");
		exit(0);
	}

	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigaction(SIGPIPE,&sa,NULL);

	sem_init(&sem,0,1);

	initItems();

	switch (argc) {
	case 2:		//ָֻ只提供了端口号
	{
		int port = atoi(argv[1]);
		if(port == 0)
		{
			printf("Usage:ftpserver [<IP Address>] <Port>\n");
			exit(1);
		}
		createconn(port, (char*) 0);
		break;
	}
	case 3:		//ָ提供了IP地址和端口号
	{
		int port = atoi(argv[2]);
		if(port == 0)
		{
			printf("Usage:ftpserver [<IP Address>] <Port>\n");
			exit(1);
		}
		char* address = argv[1];
		createconn(port, address);
		break;
	}
	default:		//用法错误
		printf("Usage:ftpserver [<IP Address>] <Port>\n");
		exit(1);
		break;
	}
	while (1) {
		readchoice();
	}
}

void readchoice(void) {	//前台读用户输入
	printf("ftpserver>");
	char buffer[BUFLENGTH];
	if (fgets(buffer, BUFLENGTH, stdin) != 0) {
		if (strcmp(buffer, "quit\n") == 0) {
			int i;
			for(i = 0;i < CONNECTION;i++)
			{
				session_end(items[i].socket);
			}
			close(server_socket);
			sem_destroy(&sem);
			printf("Goodbye.\n");
			exit(0);
		}
		else if (strcmp(buffer, "count current\n") == 0) {
			printf("The number of online users is %d.\n",count_curr(false));
		}
		else if (strcmp(buffer, "count all\n") == 0) {
			count_all();
		}
		else if(strcmp(buffer,"list\n") == 0)
		{
			listuser();
		}
		else if (strncmp(buffer, "kill ", 5) == 0) {
			char* username = &buffer[5];
			if (username[strlen(username) - 1] == '\n')
				username[strlen(username) - 1] = '\0';
			killuser(username);
		}
		else {
			printf("Sorry, wrong command.\n");
		}
	}
	return;
}


