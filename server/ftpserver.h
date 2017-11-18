#ifndef _REENTRANT
#define _REENTRANT
#endif

#ifndef _FTPSERVER_H
#define _FTPSERVER_H

#include "ftppublic.h"
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>

/**********常量宏定义************/
#define CONNECTION 5
/*******常量宏定义 END***********/

/*********数据结构**************/
//存放用户信息的结构
typedef struct {
	uid_t uid;
	int socket;
	pthread_t thread;
	char curr_dir[BUFLENGTH];
	transmode t_mode;
} useritem;

/**********全局变量************/
extern sem_t sem;				//定义在usercommand.c内
extern useritem items[CONNECTION];	//定义在server_back.c内
/*******全局变量 END**********/

/*******函数**************/
//前台函数
void readchoice(void);

//用户操作有关的函数
void initItems(void);
void deleteitem(int position);
int count_curr(bool willshow);
void count_all(void);
void listuser(void);
void killuser(char* username);
int adduser(useritem* newitem);
void session_end(int socket);
int findposition(int socket);

//后台函数
void createconn(int port,char* address);
void* startlisten(void*);

//处理用户请求的函数
void* user_session_main(void* socket);
int identify(char* id_str);

//处理用户命令的函数
void processcommand(int socket,char cmdnum);
void process_noarg(int socket,char cmdnum,char buffer[]);
void process_1arg(int socket, char cmdnum, char buffer[]);
void listfile(int socket);

//收发文件
int recvfile_server(int socket, const char* filename,const char* path, transmode tmode);
void put_server(int socket);
int sendfile_server(int socket, char* buffer);
void get_server(int socket);
/***********函数 END*********/

#endif
