#include "ftppublic.h"
#include "ftpserver.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <semaphore.h>
#include <string.h>
#include <grp.h>
#include <stdlib.h>

sem_t sem;

void processcommand(int socket, char cmdnum) {
	switch (cmdnum) {
	case PWD:
	case BINARY:
	case ASCII: {
		char buffer[BUFLENGTH];
		process_noarg(socket, cmdnum, buffer);
		write(socket, buffer, BUFLENGTH);
		break;
	}
	case MKDIR:
	case RMDIR:
	case CD: {
		char buffer[BUFLENGTH];
		if (read(socket, buffer, BUFLENGTH) <= 0) {
			session_end(socket);
			return;
		}
		process_1arg(socket, cmdnum, buffer);
		write(socket, buffer, BUFLENGTH);
		break;
	}
	case PUT: {
		put_server(socket);
		break;
	}
	case LS: {
		listfile(socket);
		break;
	}
	case GET:{
		get_server(socket);
		break;
	}
	}
}

void process_noarg(int socket, char cmdnum, char buffer[]) {
	int position = findposition(socket);

	switch (cmdnum) {
	case PWD: {
		sprintf(buffer, "Current working directory:%s",
				items[position].curr_dir);
		break;
	}
	case BINARY: {
		items[position].t_mode = tmode_binary;
		sprintf(buffer, "Transferring mode has been switched to BINARY.");
		break;
	}
	case ASCII: {
		items[position].t_mode = tmode_ascii;
		sprintf(buffer, "Transferring mode has been switched to ASCII.");
		break;
	}
	}
}

void process_1arg(int socket, char cmdnum, char buffer[]) {
	int position = findposition(socket);

	switch (cmdnum) {
	case MKDIR: {
		sem_wait(&sem);
		chdir(items[position].curr_dir);
		char* param = strdup(buffer);
		setegid(getgrnam("ftp")->gr_gid);
		seteuid(items[position].uid);
		if (mkdir(param, DEFAULTMODE_DIR) == 0) {
			chmod(param, 00755);
			sprintf(buffer, "Successfully created directory %s.", param);
		} else {
			sprintf(buffer, "Failed to create directory %s", param);
		}
		seteuid(0);
		setegid(0);
		free(param);
		sem_post(&sem);
		break;
	}
	case RMDIR: {
		sem_wait(&sem);
		chdir(items[position].curr_dir);
		char* param = strdup(buffer);
		setegid(getgrnam("ftp")->gr_gid);
		seteuid(items[position].uid);
		if (rmdir(param) == 0) {
			sprintf(buffer, "Successfully removed empty directory %s.", param);
		} else {
			sprintf(buffer, "Failed to remove directory %s", param);
		}
		seteuid(0);
		setegid(0);
		free(param);
		sem_post(&sem);
		break;
	}
	case CD: {
		sem_wait(&sem);
		chdir(items[position].curr_dir);
		char* param = strdup(buffer);
		setegid(getgrnam("ftp")->gr_gid);
		seteuid(items[position].uid);
		if (chdir(param) == 0) {
			getcwd(items[position].curr_dir,BUFLENGTH);
			sprintf(buffer, "Current working directory:%s",
							items[position].curr_dir);
		} else {
			sprintf(buffer, "Failed to change working directory.");
		}
		seteuid(0);
		setegid(0);
		free(param);
		sem_post(&sem);
		break;
		break;
	}
	}
}

void listfile(int socket) {
	int position = findposition(socket);

	char numberbuffer[BUFLENGTH];
	char lsbuffer[1024*512];		//512KB用于存放ls的结果
	memset(lsbuffer,'\0',1024*512);
	DIR* dir = opendir(items[position].curr_dir);
	memset(lsbuffer,'\0',1024*512);
	struct dirent* curritem;
	while((curritem = readdir(dir)) != NULL) {
		if(strncmp(curritem->d_name,".",1) == 0) {
			continue;
		}
		strcat(lsbuffer,curritem->d_name);
		strcat(lsbuffer,"  ");
	}
	closedir(dir);
	sprintf(numberbuffer,"%d",strlen(lsbuffer)+1);
	write(socket,numberbuffer,BUFLENGTH);
	write(socket,lsbuffer,strlen(lsbuffer)+1);
}
