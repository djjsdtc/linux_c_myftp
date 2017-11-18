#include "ftppublic.h"
#include "ftpserver.h"
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

int all_count = 0;
useritem items[CONNECTION];

void initItems(void)
{
	int i;
	for(i = 0;i < CONNECTION;i++)
	{
		deleteitem(i);
	}
}

void deleteitem(int position)
{
	items[position].uid = 0;
	items[position].socket = 0;
	items[position].thread = 0;
	strcpy(items[position].curr_dir,"");
	items[position].t_mode = tmode_binary;
}

int count_curr(bool willshow)
{
	int i;
	int total = 0;
	for(i = 0;i < CONNECTION;i++)
	{
		if(items[i].uid != 0)
		{
			total++;
			if(willshow) printf("%s\n",getpwuid(items[i].uid)->pw_name);
		}
	}
	return total;
}

void count_all(void)
{
	printf("Total number of users is %d.\n", all_count);
	return;
}

void listuser(void)
{
	if(count_curr(true) == 0) printf("No users online now.\n");
}

void killuser(char* username)
{
	struct passwd* user = getpwnam(username);
	if(user == NULL)
	{
		printf("User not exist in your system, please check.\n");
		return;
	}
	int position;
	for(position = 0;position < CONNECTION;position++)
	{
		if(items[position].uid == user->pw_uid) break;
	}
	if(position == CONNECTION)
	{
		printf("User not exist in the current user list, please check.\n");
	}
	else
	{
		session_end(items[position].socket);
		printf("User %s has been killed.\n", username);
	}
	return;
}

int adduser(useritem* newitem)
{
	int position;
	for(position = 0;position < CONNECTION;position++)
	{
		if(items[position].uid == 0) break;
	}
	items[position].uid = newitem->uid;
	items[position].socket = newitem->socket;
	items[position].thread = newitem->thread;
	strcpy(items[position].curr_dir,newitem->curr_dir);
	items[position].t_mode = tmode_binary;
	all_count++;
	return count_curr(false);
}

void session_end(int socket)
{
	sem_wait(&sem);
	int position = findposition(socket);
	pthread_cancel(items[position].thread);
	close(socket);
	deleteitem(position);
	sem_post(&sem);
}

int findposition(int socket)
{
	int position;
	for (position = 0; position < CONNECTION; position++) {
		if (items[position].socket == socket)
			break;
	}
	return position;
}
