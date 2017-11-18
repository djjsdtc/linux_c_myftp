#include "myftp_header.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc,char* argv[])
{
	if(argc!=2) error_usage();

	printf("myftp\n");

	userinfo infos;
	getparams(argv[1],&infos);

	printf("Connecting to %s:%d.\n\n",infos.ipaddress,infos.port);

	int sockid = createconn(infos.ipaddress,infos.port);
	int retValue = identify(sockid,infos.id_str);
	if(retValue == -1)
	{
		fprintf(stderr,"Username does not exist, or password is incorrect.\n");
		exit(1);
	}
	else
	{
		printf("You are client %d.\n",retValue);
	}
	tmode = tmode_binary;
	while(1)
	{
		getcommand(sockid);
	}

	return 0;
}

void getparams(char* arg,userinfo* infoptr)
{
	char* id_str = arg;

	char* pw_ip_sep = strrchr(id_str,'@');		//密码、IP地址分隔符位置
	if(pw_ip_sep == NULL) error_usage();
	char* ipadd = pw_ip_sep + 1;
	*pw_ip_sep = '\0';

	char* ip_pt_sep = strrchr(ipadd,':');		//IP地址、端口分隔符位置
	if(ip_pt_sep == NULL) error_usage();
	char* port_str = ip_pt_sep + 1;
	int port = atoi(port_str);
	*ip_pt_sep = '\0';

	infoptr->id_str = id_str;
	infoptr->ipaddress = ipadd;
	infoptr->port = port;

	return;
}

void error_usage(void)
{
	fprintf(stderr,"Usage:myftp <username>:<password>@<server_ip>:<server_port>\n");
	exit(1);
}
