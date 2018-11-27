#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>

#define REG 0		//注册
#define LOGIN 1		//登陆
#define FORGETPW 2	//重设密码
#define MANAGE 3	//管理
#define G_CHAT 4	//群聊
#define P_CHAT 5	//私聊
#define SENDFILE 6	//文件
#define EXIT 7		//注销
#define ACK 8		

#define BUFFSIZE 1024
#define NAMESIZE 20
#define BACKLOG 10
#define SERIP 127.0.0.1
#define PORT 2500

struct msg{
	int type;
	int len;
	char object[NAMESIZE];
	char data[BUFFSIZE];
};

struct user{
	char name[NAMESIZE];
	char pw[NAMESIZE];
	char question[100];
	char answer[100];
	int p_connfd;
	struct user *next;
};

int Socket(int domain, int type, int protocol)
{
	int sockfd;
	sockfd = socket(domain, type, protocol);
	if(sockfd < 0)
	{
		perror("socket");
		exit(-1);
	}
	printf("sockfd success.\nsockfd = %d\n", sockfd);
	return sockfd;
}
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	if(bind(sockfd, addr, addrlen) == -1)
	{
		perror("bind");
		exit(-1);
	}
	printf("bind success.\n");
	return 0;
}
int Listen(int sockfd, int backlog)
{
	if(listen(sockfd, backlog) == -1)
	{
		perror("listen");
		exit(-1);
	}
	printf("listen success.\n");
	return 0;
}
int Accept(int sockfd, struct sockaddr *address, socklen_t *address_len)
{
	int newfd;
	newfd = accept(sockfd, address, address_len);
	if(newfd == -1)
	{
		perror("accept");
		exit(-1);
	}
	printf("listen success.\n");
	return newfd;
}
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	if(connect(sockfd,  addr, addrlen) == -1)
	{
		perror("connect");
		exit(-1);
	}
	printf("connect success.\n");
	return 0;
}
