#include "ftppublic.h"
#include "myftp_header.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int sendfile(int socket, char* buffer) {
	//-2表示文件错误，无需退出程序；-1表示网络错误，客户端退出程序。
	//首先打开文件，如果打不开，put操作直接返回-2，get操作下发提示并返回-2
	FILE* fromfile = fopen(buffer, "r");
	if (fromfile == NULL ) {
		char startValue = FILEFAULT;
		write(socket, &startValue, sizeof(char));

		printf("Cannot read file: %s\n", buffer);
		return -2;
	}
	/* 1.向接收方发送请求传输文件的指令。
	 * 指令名为宏定义CANTRANSFER。
	 * 后加文件名。
	 */
	//发送头
	char startValue = CANTRANSFER;
	write(socket, &startValue, sizeof(char));
	//发送文件名，buffer是一个BUFLENGTH大小的缓冲区
	//发送的文件名不应该包含全路径
	char filename[BUFLENGTH];
	char* fullpath_indicator = strrchr(buffer, '/');
	if(fullpath_indicator == NULL) strcpy(filename,buffer);
	else strcpy(filename, fullpath_indicator + 1);
	write(socket, filename, BUFLENGTH);
	while (1) {
		/* 2.等待接收方发回确认，以及现在已传多少
		 * 如果收不到确认或者不允许传送文件，返回-1表示出错。
		 * 如果收到确认且和文件大小一致，表示传输完毕。
		 */
		char returnValue;
		if (read(socket, &returnValue, sizeof(char)) <= 0) {
			printf("Connection lost.\n");
			fclose(fromfile);
			return -1;
		} else if (returnValue == FILEFAULT) {
			printf("The server cannot create this file.\n");
			fclose(fromfile);
			return -2;
		} else {		//DATARECEIVED
			if (read(socket, buffer, BUFLENGTH) <= 0) {
				printf("Connection lost.\n");
				fclose(fromfile);
				return -1;
			} else {
				long receivedSize = atol(buffer);
				/* 3.发文件，然后继续到下一路循环。 */
				fseek(fromfile, receivedSize, SEEK_SET);
				char sendBuffer[1048576];		//以1MB为一块
				int actualsize;
				//要知道实际上究竟要发送多少
				actualsize = (int) fread(sendBuffer, 1, 1048576, fromfile);
				if (actualsize == 0 && feof(fromfile)) {
					char endValue = FILEDONE;
					write(socket, &endValue, sizeof(char));
					char getEndValue;
					if (read(socket, &getEndValue, sizeof(char)) <= 0) {
						printf("Connection lost.\n");
						fclose(fromfile);
						return -1;
					} else {
						printf("Completely sended file.%ld bytes sent.\n",
								receivedSize);
					}
					fclose(fromfile);
					return 0;
				}
				//先给服务器一个DATA信号
				char signalValue = DATA;
				write(socket, &signalValue, sizeof(char));
				//再给服务器发一个实际发送的文件大小
				sprintf(buffer, "%d", actualsize);
				write(socket, buffer, BUFLENGTH);
				//再发送数据块过去。
				write(socket, sendBuffer, actualsize);

				if (ftell(fromfile) % 10485760 == 0) {
					printf("%ld bytes sent.", ftell(fromfile));
				}		//IF
			}		//ELSE
		}		//ELSE
	}		//WHILE
	return 0;
}

void put_client(int socket,char* filename){
	//发送put命令
	char startValue = PUT;
	write(socket,&startValue,sizeof(char));
	//等待服务器返回FILEDONE，如果返回FILEFAULT则报错退出
	if(read(socket,&startValue,sizeof(char)) <= 0){
		printf("Connection lost.\n");
		close(socket);
		exit(0);
	} else if(startValue == FILEFAULT) {
		printf("You cannot send to this folder due to permission.\n");
		return;
	}
	//如果是FILEDONE则进入函数
	printf("Sending file %s\n",filename);
	printf("Current transferring mode is ");
	if(tmode == tmode_ascii) printf("ASCII mode.\n");
	else printf("BINARY mode.\n");
	if(sendfile(socket,filename) == -1){
		close(socket);
		exit(0);
	}
}
