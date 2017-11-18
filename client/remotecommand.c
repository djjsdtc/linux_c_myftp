#include "myftp_header.h"
#include "ftppublic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool remote_noarg(int socket,char buffer[])
{
	char command;
	if(strcmp(buffer,"pwd\n") == 0)
	{
		command = PWD;
	}
	else if(strcmp(buffer,"ls\n") == 0)
	{
		command = LS;
		write(socket,&command,sizeof(char));
		//读数据大小
		if(read(socket,buffer,BUFLENGTH) <= 0){
			printf("Connection lost.\n");
			close(socket);
			exit(0);
		}
		int buflength = atoi(buffer);
		char* lsbuffer = (char*)malloc(buflength);
		if(read(socket,lsbuffer,buflength) <= 0){
			printf("Connection lost.\n");
			close(socket);
			exit(0);
		}
		printf("[R]%s\n",lsbuffer);
		free(lsbuffer);
		return true;
	}
	else if(strcmp(buffer,"binary\n") == 0)
	{
		command = BINARY;
		tmode = tmode_binary;
	}
	else if(strcmp(buffer,"ascii\n") == 0)
	{
		command = ASCII;
		tmode = tmode_ascii;
	}
	else
	{
		return false;
	}
	write(socket,&command,sizeof(char));
	if(read(socket,buffer,BUFLENGTH) <= 0){
		printf("Connection lost.\n");
		close(socket);
		exit(0);
	}
	printf("[R]%s\n",buffer);
	return true;
}

bool remote_1arg(int socket,char buffer[])
{
	char command;
	char param[BUFLENGTH];
	if(strncmp(buffer,"mkdir ",6) == 0)
	{
		strcpy(param,&buffer[6]);
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = '\0';
		command = MKDIR;
	}
	else if(strncmp(buffer,"rmdir ",6) == 0)
	{
		strcpy(param,&buffer[6]);
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = '\0';
		command = RMDIR;
	}
	else if(strncmp(buffer,"cd ",3) == 0)
	{
		strcpy(param,&buffer[3]);
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = '\0';
		command = CD;
	}
	else if(strncmp(buffer,"put ",4) == 0)
	{
		strcpy(param,&buffer[4]);
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = '\0';
		put_client(socket,param);
		return true;
	}
	else if(strncmp(buffer,"get ",4) == 0)
	{
		strcpy(param,&buffer[4]);
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = '\0';
		get_client(socket,param);
		return true;
	}
	else
	{
		return false;
	}
	write(socket,&command,sizeof(char));
	write(socket,param,BUFLENGTH);
	if(read(socket,buffer,BUFLENGTH) <= 0){
		printf("Connection lost.\n");
		close(socket);
		exit(0);
	}
	printf("[R]%s\n",buffer);
	return true;
}

bool remote_marg(int socket,char buffer[])
{
	if(strncmp(buffer,"mput ",5) == 0)
	{
		char* param = &buffer[5];
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = ' ';
		char* position;
		while((position = strchr(param,' ')) != NULL)
		{
			char* nextparam = position + 1;
			*position = '\0';
			//做put操作
			char filename[BUFLENGTH];
			strcpy(filename,param);
			put_client(socket,filename);
			//读下一个文件名
			param = nextparam;
		}
	}
	else if(strncmp(buffer,"mget ",5) == 0)
	{
		char* param = &buffer[5];
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = ' ';
		char* position;
		while((position = strchr(param,' ')) != NULL)
		{
			char* nextparam = position + 1;
			*position = '\0';
			//做get操作
			char filename[BUFLENGTH];
			strcpy(filename,param);
			get_client(socket,filename);
			//读下一个文件名
			param = nextparam;
		}
	}
	else
	{
		return false;
	}
	return true;
}
