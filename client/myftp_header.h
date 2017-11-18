#ifndef MYFTP_HEADER_H_
#define MYFTP_HEADER_H_

#include "ftppublic.h"

/***********数据结构************/
typedef struct
{
	char* id_str;
	char* ipaddress;
	int port;
} userinfo;
/********数据结构 END***********/

//传输方式
extern transmode tmode;

/**********基本函数************/
int createconn(char* address,int port);
void getparams(char* arg,userinfo* infoptr);
void error_usage(void);
void getcommand(int socket);
int identify(int socket,char* id_str);
/*********基本函数 END*********/

/**********命令解释************/
bool preprocess(char buffer[]);
bool localcommands(int socket,char buffer[]);
bool remote_noarg(int socket,char buffer[]);
bool remote_1arg(int socket,char buffer[]);
bool remote_marg(int socket,char buffer[]);
/*********命令解释 END*********/

/************传文件***********/
int sendfile(int socket, char* buffer);
void put_client(int socket,char* filename);
int recvfile(int socket, const char* filename, transmode tmode);
void get_client(int socket,char* filename);
/*********传文件 END**********/
#endif
