#define _GNU_SOURCE

#include "ftppublic.h"
#include "ftpserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <grp.h>

int sendfile_server(int socket, char* buffer) {
	//服务器端get专用发送函数，去掉所有的输出
	//返回值：-2表示文件错误，-1表示网络错误
	//首先打开文件
	FILE* fromfile = fopen(buffer, "r");
	if (fromfile == NULL ) {
		char startValue = FILEFAULT;
		write(socket, &startValue, sizeof(char));
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
			fclose(fromfile);
			return -1;
		} else if (returnValue == FILEFAULT) {
			fclose(fromfile);
			return -2;
		} else {		//DATARECEIVED
			if (read(socket, buffer, BUFLENGTH) <= 0) {
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
						fclose(fromfile);
						return -1;
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
			}
		}
	}
	return 0;
}

void get_server(int socket) {
	int position;
	for (position = 0; position < CONNECTION; position++) {
		if (items[position].socket == socket)
			break;
	}
	//首先获取文件名
	char filename[BUFLENGTH];
	if (read(socket, filename, BUFLENGTH) <= 0) {
		session_end(socket);
		return;
	}
	//然后测试读取权限
	sem_wait(&sem);
	chdir(items[position].curr_dir);
	setegid(getgrnam("ftp")->gr_gid);
	seteuid(items[position].uid);
	if(eaccess(filename,R_OK) != 0) {
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
	//如果可读，进函数进行发送文件
	if(sendfile_server(socket,filename) == -1) {
		//网络错误
		session_end(socket);
	}
}
