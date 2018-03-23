/*
 * server.c
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "http_work.h"


char LOGBUF[1024];

int main(void)
{
	int port = 8090;
	int client_sock=-1;
	int server_sock=-1;
	struct sockaddr_in client_sockaddr;
	socklen_t client_len = sizeof(client_sockaddr);
	pthread_t newthread;
	
	server_sock=socket_create(port);//建立socket，监听端口
	while (1)
	{
		//端口监听http请求
		client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, &client_len);
		if (client_sock == -1)
		{
			memset(LOGBUF,0,sizeof(LOGBUF));
			sprintf(LOGBUF,"%s,%d:accept failture %s \n", __FILE__, __LINE__,
					strerror(errno));
			save_log(LOGBUF);
			return 0;
		} else//子线程处理http请求
		{
			int *tmp = (int *) malloc(sizeof(int));
			*tmp = client_sock;
			pthread_create(&newthread, NULL, http_thread, tmp);
		}
	}
	
	close(server_sock);
	return 0;
}