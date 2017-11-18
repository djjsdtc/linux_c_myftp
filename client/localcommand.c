#include "myftp_header.h"
#include "ftppublic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

void getcommand(int socket)
{
	printf("myftp>");
	char buffer[BUFLENGTH];
	if(fgets(buffer,BUFLENGTH,stdin) != 0)
	{
		if(preprocess(buffer)){}
		else if(localcommands(socket,buffer)){}
		else if(remote_noarg(socket,buffer)){}
		else if(remote_1arg(socket,buffer)){}
		else if(remote_marg(socket,buffer)){}
		else
		{
			printf("Invalid command. Try again.\n");
		}
	}
	return;
}

bool preprocess(char buffer[])
{
	//常见错误命令的预处理
	if(strcmp(buffer,"lmkdir\n") == 0 || strcmp(buffer,"lmkdir \n") == 0)
	{
		printf("Usage:lmkdir <local directory name>\n");
	}
	else if(strcmp(buffer,"lrmdir\n") == 0 || strcmp(buffer,"lrmdir \n") == 0)
	{
		printf("Usage:lrmdir <local directory name>\n");
	}
	else if(strcmp(buffer,"lcd\n") == 0 || strcmp(buffer,"lcd \n") == 0)
	{
		printf("Usage:lcd <local directory name>\n");
	}
	else if(strcmp(buffer,"mkdir\n") == 0 || strcmp(buffer,"mkdir \n") == 0)
	{
		printf("Usage:mkdir <remote directory name>\n");
	}
	else if(strcmp(buffer,"rmdir\n") == 0 || strcmp(buffer,"rmdir \n") == 0)
	{
		printf("Usage:rmdir <remote directory name>\n");
	}
	else if(strcmp(buffer,"cd\n") == 0 || strcmp(buffer,"cd \n") == 0)
	{
		printf("Usage:cd <remote directory name>\n");
	}
	else if(strcmp(buffer,"put\n") == 0 || strcmp(buffer,"put \n") == 0)
	{
		printf("Usage:put <local file name>\n");
	}
	else if(strcmp(buffer,"get\n") == 0 || strcmp(buffer,"get \n") == 0)
	{
		printf("Usage:get <remote file name>\n");
	}
	else if(strcmp(buffer,"mput\n") == 0 || strcmp(buffer,"mput \n") == 0)
	{
		printf("Usage:mput <file1> <file2> <...>\n");
	}
	else if(strcmp(buffer,"mget\n") == 0 || strcmp(buffer,"mget \n") == 0)
	{
		printf("Usage:mget <file1> <file2> <...>\n");
	}
	else
	{
		return false;
	}
	return true;
}

bool localcommands(int socket,char buffer[])
{
	if(strcmp(buffer,"quit\n") == 0)
	{
		close(socket);
		printf("Goodbye.\n");
		exit(0);
	}
	else if(strncmp(buffer,"lmkdir ",7) == 0)
	{
		char* param = &buffer[7];
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = '\0';
		if(mkdir(param,DEFAULTMODE_DIR) != 0)
		{
			printf("[L]Failed to create directory: %s\n",strerror(errno));
		}
		else
		{
			printf("[L]Successfully created directory:%s\n",param);
		}
	}
	else if(strncmp(buffer,"lrmdir ",7) == 0)
	{
		char* param = &buffer[7];
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = '\0';
		if(rmdir(param) != 0)
		{
			printf("[L]Failed to remove directory: %s\n",strerror(errno));
		}
		else
		{
			printf("[L]Successfully removed empty directory:%s\n",param);
		}
	}
	else if(strcmp(buffer,"lpwd\n") == 0)
	{
		printf("[L]Current working directory:%s\n",getcwd(buffer,BUFLENGTH));
	}
	else if(strncmp(buffer,"lcd ",4) == 0)
	{
		char* param = &buffer[4];
		if (param[strlen(param) - 1] == '\n')
			param[strlen(param) - 1] = '\0';
		if(chdir(param) != 0)
		{
			printf("[L]Failed to change working directory: %s\n",strerror(errno));
		}
		else
		{
			printf("[L]Current working directory:%s\n",getcwd(buffer,BUFLENGTH));
		}
	}
	else if(strcmp(buffer,"dir\n") == 0)
	{
		DIR* dir = opendir(getcwd(buffer,BUFLENGTH));
		struct dirent* dircontext = readdir(dir);
		int i = 1;
		while(dircontext != NULL)
		{
			if(strncmp(dircontext->d_name,".",1) != 0)
			{
				printf("%s\t",dircontext->d_name);
				if(++i % 6 == 0) printf("\n");
			}
			dircontext = readdir(dir);
		}
		if(i % 6 != 0) printf("\n");
		if(i == 1) printf("[L]Current directory is empty.\n");
		closedir(dir);
	}
	else
	{
		return false;
	}
	return true;
}
