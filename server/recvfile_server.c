#define _GNU_SOURCE

#include "ftppublic.h"
#include "ftpserver.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <grp.h>

int recvfile_server(int socket, const char* filename, const char* path, transmode tmode) {
	//服务器端put专用版收文件函数，去掉原函数中所有的输出。
	//返回值：-2表示文件错误，-1表示网络错误。
	char tempname[BUFLENGTH];
	strcpy(tempname, ".");
	strcat(tempname, filename);
	sem_wait(&sem);
	chdir(path);
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

		if (tempfp != NULL ){
			fclose(tempfp);
			unlink(tempname);
		}
		if (tofile != NULL ) {
			fclose(tofile);
		}
		sem_post(&sem);
		return -2;
	} else {
		//无文件，无隐藏文件，正好打开了写入模式。
		fprintf(tempfp, "0");
		fclose(tempfp);
	}
	sem_post(&sem);
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
			fclose(tofile);
			return -1;
		} else if (receivedValue == FILEFAULT) {
			//如果收到客户端文件读取错误
			fclose(tofile);
			return -2;
		} else if (receivedValue == FILEDONE) {
			//如果接收到文件已完毕信号
			char returnValue = FILEDONE;
			write(socket, &returnValue, sizeof(char));
			fclose(tofile);
			unlink(tempname);
			return 0;
		} else {
			//先接收要发的文件大小
			if (read(socket, buffer, BUFLENGTH) <= 0) {
				fclose(tofile);
				return -1;
			}
			int toreceive_size = atoi(buffer);
			//申请内存空间
			char* receiveBuffer = (char*) malloc(toreceive_size);
			//接收数据
			if (read(socket, receiveBuffer, toreceive_size) <= 0) {
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
		}
	}
	return 0;
}

void put_server(int socket) {
	int position = findposition(socket);
	char buffer[BUFLENGTH];
	char* filename = NULL;
	//进行权限测试：如果用户对当前的文件夹有写权限则通过，否则权限不足
	sem_wait(&sem);
	setegid(getgrnam("ftp")->gr_gid);
	seteuid(items[position].uid);
	if(eaccess(items[position].curr_dir,W_OK) != 0) {
		char retValue = FILEFAULT;
		write(socket,&retValue,sizeof(char));
		seteuid(0);
		setegid(0);
		sem_post(&sem);
		return;
	}
	seteuid(0);
	setegid(0);
	sem_post(&sem);
	//如果允许写权限，则开始接收文件，文件名由CANTRANSFER的参数指定
	//首先发送FILEDONE信号，然后如果对方回传CANTRANSFER则进入流程
	//如果对方回发FILEFAULT则结束过程
	char retValue = FILEDONE;
	write(socket,&retValue,sizeof(char));
	if (read(socket, &retValue, sizeof(char)) <= 0) {
		session_end(socket);
		return;
	}
	if(retValue == CANTRANSFER){
		if (read(socket, buffer, BUFLENGTH) <= 0) {
			session_end(socket);
			return;
		}
		filename = strdup(buffer);
		if(recvfile_server(socket,buffer,items[position].curr_dir,items[position].t_mode) == -1) {
			//网络错误
			session_end(socket);
			return;
		}
	}
	//最后更改文件的UID和GID到本用户
	sem_wait(&sem);
	chdir(items[position].curr_dir);
	chown(filename,items[position].uid,getgrnam("ftp")->gr_gid);
	if(filename != NULL) free(filename);
	sem_post(&sem);
}
