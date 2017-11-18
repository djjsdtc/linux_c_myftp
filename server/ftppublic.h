#ifndef _FTPPUBLIC_H
#define _FTPPUBLIC_H

//bool类型定义
static const int true = 1;
static const int false = 0;
typedef int bool;

//命令代码定义
#define MKDIR 0
#define RMDIR 1
#define PWD 2
#define CD 3
#define LS 4
#define PUT 5
#define GET 6
#define BINARY 7
#define ASCII 8
#define DATA 9

//命令返回值定义
#define CANTRANSFER 10
#define FILEFAULT 11
#define DATARECEIVED 12
#define FILEDONE 13

//传输模式
typedef enum {tmode_binary, tmode_ascii} transmode;

//文件模式
#define DEFAULTMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH

//目录模式
#define DEFAULTMODE_DIR S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH

//缓冲区大小
#define BUFLENGTH 256

#endif
