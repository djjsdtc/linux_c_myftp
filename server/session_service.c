#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include "ftppublic.h"
#include "ftpserver.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <shadow.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <pthread.h>
#include <semaphore.h>

void* user_session_main(void* socket)
{
	int sockid = (int)socket;
	char tosend = 0;
	write(sockid,&tosend,sizeof(char));
	char buffer[BUFLENGTH];
	read(sockid,buffer,BUFLENGTH);	//读取username:password格式的用户名和密码
	int retValue = identify(buffer);
	if(retValue == -1)		//retValue为-1表示身份验证失败
	{
		tosend = -1;
		write(sockid,&tosend,sizeof(char));
		close(sockid);
		return (void*)0;
	}
	else
	{
		useritem item;
		item.socket = sockid;
		item.thread = pthread_self();
		item.uid = (uid_t)retValue;
		char* userhome = NULL;
		userhome = getpwuid(item.uid)->pw_dir;
		strcpy(item.curr_dir,userhome);
		char order = (char)adduser(&item);
		write(sockid,&order,sizeof(char));
	}
	while(1)
	{
		char cmdnum;
		if(read(sockid,&cmdnum,sizeof(char)) <= 0)
		{
			break;
		}
		else
		{
			processcommand(sockid,cmdnum);
		}
	}
	int position = findposition(sockid);
	close(sockid);
	deleteitem(position);
	return (void*)0;
}

int identify(char* id_str)
{
	//返回用户UID表示成功，-1表示失败。
	sem_wait(&sem);
	char* name = id_str;
	char* nm_pw_sep = strchr(id_str,':');		//用户名、密码分隔符位置
	if(nm_pw_sep == NULL) return -1;
	*nm_pw_sep = '\0';
	struct spwd* myuser = getspnam(name);
	if((myuser == NULL) || (getpwnam(name)->pw_gid != getgrnam("ftp")->gr_gid))
	{
		sem_post(&sem);
		return -1;
	}
	else
	{
		char* pswd = nm_pw_sep + 1;
		char* en_pswd = NULL;
		en_pswd = crypt(pswd,myuser->sp_pwdp);
		if(strcmp(myuser->sp_pwdp,en_pswd)==0)
		{
			sem_post(&sem);
			return (int)getpwnam(name)->pw_uid;
		}
		else
		{
			sem_post(&sem);
			return -1;
		}
	}
}
