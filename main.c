#include "http.h"
static void usage(const char* proc)
{
	printf("Usage: %s[local_ip] [local_port]\n",proc);
}
int main(int argc,char *argv[])
{
	if(argc!=3)
	{
		usage(argv[0]);
		return 1;
	}
	int listen_sock=startup(argv[1],atoi(argv[2]));
	if(listen_sock<0)
	{
		return 2;
	}
	while(1)
	{
		struct sockaddr_in client;
		socklen_t len = sizeof(client);
		int new_sock=accept(listen_sock,(struct sockaddr*)&client,&len);
		if(new_sock<0)
		{
			perror("accept");
			continue;
		}
		printf("get a client:[%s:%d]\n",inet_ntoa(client.sin_addr),\
				ntohs(client.sin_port));
		pthread_t id;
		pthread_create(&id,NULL,handler_request,(void*)new_sock);
		pthread_detach(id);
	}
	return 0;
}
