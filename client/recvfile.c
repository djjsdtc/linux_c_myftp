#include "ftppublic.h"
#include "myftp_header.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>

transmode tmode;

int recvfile(int socket, const char* filename, transmode tmode) {
	//-2表示文件错误，无需退出程序；-1表示网络错误，客户端退出程序。
	char tempname[BUFLENGTH];
	strcpy(tempname, ".");
	strcat(tempname, filename);
	FILE* tempfp = fdopen(
			open(tempname, O_CREAT | O_WRONLY | O_EXCL, DEFAULTMODE), "w");
	FILE* tofile = fdopen(
			open(filename, O_CREAT | O_WRONLY | O_EXCL, DEFAULTMODE), "w");
	if (tempfp == NULL && tofile == NULL ) {
		//有文件，也有隐藏文件，说明是断点续传。
		tofile = fopen(filename, "a");
	} else if (tempfp == NULL || tofile == NULL ) {
		//有隐藏文件，无非隐藏文件，如“.profile”
		//或有文件，无隐藏文件
		//不允许写。
		char startValue = FILEFAULT;
		write(socket, &startValue, sizeof(char));

		printf("Cannot create file: %s\n", filename);
		if (tempfp != NULL ){
			fclose(tempfp);
			unlink(tempname);
		}
		if (tofile != NULL ) {
			fclose(tofile);
		}
		return -2;
	} else {
		//无文件，无隐藏文件，正好打开了写入模式。
		fprintf(tempfp, "0");
		fclose(tempfp);
	}
	while (1) {
		char buffer[BUFLENGTH];
		//发送当前文件大小的报头
		char startValue = DATARECEIVED;
		write(socket, &startValue, sizeof(char));
		//计算当前的文件大小
		long currentsize;
		switch (tmode) {
		case tmode_binary: {
			currentsize = ftell(tofile);
			break;
		}
		case tmode_ascii: {
			tempfp = fopen(tempname, "r");
			fgets(buffer, BUFLENGTH, tempfp);
			currentsize = atol(buffer);
			fclose(tempfp);
		}
		}
		//发送当前文件的大小
		sprintf(buffer, "%ld", currentsize);
		write(socket, buffer, BUFLENGTH);

		//接收数据段，首先接收数据段的报头。
		char receivedValue;
		if (read(socket, &receivedValue, sizeof(char)) <= 0) {
			printf("Network fault while receiving result.\n");
			fclose(tofile);
			return -1;
		} else if (receivedValue == FILEFAULT) {
			//如果收到服务端文件读取错误
			printf("The server cannot read this file.\n");
			fclose(tofile);
			return -2;
		} else if (receivedValue == FILEDONE) {
			printf("Finished receiving file.\n");
			char returnValue = FILEDONE;
			write(socket, &returnValue, sizeof(char));
			fclose(tofile);
			unlink(tempname);
			return 0;
		} else {
			//先接收要发的文件大小
			if (read(socket, buffer, BUFLENGTH) <= 0) {
				printf("Network fault while receiving result.\n");
				fclose(tofile);
				return -1;
			}
			int toreceive_size = atoi(buffer);
			//申请内存空间
			char* receiveBuffer = (char*) malloc(toreceive_size);
			//接收数据
			if (read(socket, receiveBuffer, toreceive_size) <= 0) {
				printf("Network fault while receiving data.\n");
				fclose(tofile);
				return -1;
			}
			switch (tmode) {
			case tmode_binary: {
				//直接写入
				fwrite(receiveBuffer, toreceive_size, 1, tofile);
				break;
			}
			case tmode_ascii: {
				//关于ASCII模式的客户端处理请见附件文档
				currentsize += toreceive_size;
				int i;
				for (i = 0; i < toreceive_size; i++) {
					if (receiveBuffer[i] == '\r' && receiveBuffer[i+1] == '\n')
						continue;
					else if(receiveBuffer[i] == '\r')
						fputc('\n', tofile);
					else
						fputc(receiveBuffer[i], tofile);
				}
				tempfp = fopen(tempname, "w");
				fprintf(tempfp, "%ld", currentsize);
				fclose(tempfp);
				break;
			}
			}
			free(receiveBuffer);
			if (ftell(tofile) % 10485760 == 0) {
				printf("%ld bytes received.", ftell(tofile));
			}
		}
	}
	return 0;
}

void get_client(int socket,char* filename){
	//发送get命令和文件名
	char startValue = GET;
	write(socket,&startValue,sizeof(char));
	write(socket,filename,BUFLENGTH);
	//等待服务器返回CANTRANSFER，如返回FILEFAULT则报错退出
	if(read(socket,&startValue,sizeof(char)) <= 0){
		printf("Connection lost.\n");
		close(socket);
		exit(0);
	} else if(startValue == FILEFAULT) {
		printf("Cannot get file %s: File not found or permission denied.\n",filename);
		return;
	}
	//服务器返回CANTRANSFER，首先忽略传来的文件名，然后调用函数
	if(read(socket,filename,BUFLENGTH) <= 0){
			printf("Connection lost.\n");
			close(socket);
			exit(0);
	}
	printf("Receiving file %s\n",filename);
	printf("Current transferring mode is ");
	if(tmode == tmode_ascii) printf("ASCII mode.\n");
	else printf("BINARY mode.\n");
	if(recvfile(socket,filename,tmode) == -1){
		close(socket);
		exit(0);
	}
}
