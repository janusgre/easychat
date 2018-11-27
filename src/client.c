#include "head.h"
struct myinform{
	char name[NAMESIZE];
	int fd;
};
void reg(int sockfd);
void login(int sockfd, char *name);
void forgetpw(int sockfd);
void print_main_menu();
void mySendMsg(struct msg *sm, int sockfd);
void sendMsg_file(int sockfd, char *name);
void print_manage_menu();
void sendMsg_manage(int sockfd, char *name);
void sendMsg_online(int sockfd);
void sendMsg_group(int sockfd, char *name);
void sendMsg_private(int sockfd, char *name);
void print_menu();
void *sendMsg(void *arg);
void recvMsg_FILE(int sockfd);
void *recvMsg(void *arg);
void main_menu(int sockfd);

void reg(int sockfd)
{
	struct msg *rm, *sm;
	rm = (struct msg *)malloc(sizeof(struct msg));
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	memset(rm, 0, sizeof(struct msg));
	char name[NAMESIZE], pw[NAMESIZE], pw2[NAMESIZE];
	char q[100], a[100];
	int flg = 0, recvlen = 0;
	printf("输入用户名:");
	scanf("%s", name);
	printf("您的用户名为:%s\n", name);
	while(flg == 0)
	{
		printf("请输入密码:");
		scanf("%s", pw);
		printf("请再次输入密码:");
		scanf("%s", pw2);
		if(strcmp(pw, pw2) == 0)
		{
			flg = 1;
		}
		else
		{
			printf("您两次输入的密码不一致。\n");
		}
	}
	sm->type = REG;
	strcpy(sm->object, name);
	strcpy(sm->data, pw);
	sm->len = strlen(sm->data);
	mySendMsg(sm, sockfd);
	recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
		exit(-1);
	}
	if(rm->type == ACK)
	{
		printf("%s\n", rm->data);
	}
	else
	{
		printf("出错了QAQ\n");
		free(rm);
		free(sm);
		return;
	}
	if(strncmp(rm->data, "reg success", 11) != 0)
	{
		printf("%s\n", rm->data);
		free(rm);
		free(sm);
		return;
	}
	getchar();
	printf("输入密码保护问题:");
	fgets(q, 100, stdin);
	printf("输入答案:");
	fgets(a, 100, stdin);
	a[strlen(a)-1] = 0;
	memset(sm, 0, sizeof(struct msg));
	sm->type = FORGETPW;
	sprintf(sm->data, "%s%s", q, a);
	sm->len = strlen(sm->data);
	mySendMsg(sm, sockfd);
	recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
		free(rm);
		free(sm);
		exit(-1);
	}
	printf("%s\n", rm->data);
	free(rm);
	free(sm);
	return;
}

void login(int sockfd, char *name)
{
	int ret;
	struct msg *rm, *sm;
	rm = (struct msg *)malloc(sizeof(struct msg));
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	memset(rm, 0, sizeof(struct msg));
	int recvlen = 0;
	char pw[NAMESIZE];
	printf("欢迎进入聊天室!\n");
	printf("输入用户名:");
	scanf("%s", name);
	printf("请输入密码:");
	scanf("%s", pw);

	memset(sm, 0, sizeof(struct msg));
	sm->type = LOGIN;
	strcpy(sm->object, name);
	strcpy(sm->data, pw);
	sm->len = strlen(sm->data);
	mySendMsg(sm, sockfd);
	memset(rm, 0, sizeof(struct msg));
	recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
		exit(-1);
	}
	if(rm->type == ACK)
		printf("%s\n", rm->data);
	else
	{
		printf("出错了QAQ\n");
		memset(name, 0, sizeof(char) * NAMESIZE);
		return;
	}
	if(strncmp(rm->data, "login success", 13) == 0)
	{
		return;
	}
	free(rm);
	free(sm);
	memset(name, 0, sizeof(char) * NAMESIZE);
	return;
}
void forgetpw(int sockfd)
{
	int ret;
	struct msg *rm, *sm;
	rm = (struct msg *)malloc(sizeof(struct msg));
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	memset(rm, 0, sizeof(struct msg));
	char name[NAMESIZE], pw[NAMESIZE], pw2[NAMESIZE];
	char answer[100];
	int flg = 0, recvlen = 0;
	printf("输入用户名:");
	scanf("%s", name);
	memset(sm, 0, sizeof(struct msg));
	sm->type = FORGETPW;
	strcpy(sm->data, name);
	mySendMsg(sm, sockfd);
	memset(rm, 0, sizeof(struct msg));
	recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
		exit(-1);
	}
	if(rm->type == ACK && strncmp(rm->object, "question", 8) == 0)
	{
		printf("%s", rm->data);
	}
	else
	{
		printf("%s\n", rm->data);
		free(rm);
		free(sm);
		return;
	}
	getchar();
	fgets(sm->data, 100, stdin);
	sm->data[strlen(sm->data)-1] = 0;
	mySendMsg(sm, sockfd);
	memset(rm, 0, sizeof(struct msg));
	recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
	}
	if(strncmp(rm->data, "correct", 7) == 0)
	{
		printf("回答正确。\n");
	}
	else
	{
		printf("回答错误。\n");
		free(rm);
		free(sm);
		return;
	}
	while(flg == 0)
	{
		printf("请输入密码:");
		scanf("%s", pw);
		printf("请再次输入密码:");
		scanf("%s", pw2);
		if(strcmp(pw, pw2) == 0)
		{
			flg = 1;
		}
		else
		{
			printf("您两次输入的密码不一致。\n");
		}
	}
	strcpy(sm->data, pw);
	sm->len = strlen(sm->data);
	mySendMsg(sm, sockfd);
	recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
	}
	if(rm->type == ACK)
	{
		printf("%s\n", rm->data);
	}
	else
	{
		printf("出错了QAQ\n");
	}
	free(rm);
	free(sm);
	return;
}
void print_main_menu()
{
	printf("*********菜单*********\n");
	printf("1.登陆       2.注册\n");
	printf("3.忘记密码   4.退出\n");
	return;
}
void mySendMsg(struct msg *sm, int sockfd)
{
	int ret;
	ret = send(sockfd, sm, sizeof(struct msg), 0);
	if(ret <= 0)
	{
		perror("send");
	}
	return;
}
void sendMsg_file(int sockfd, char *name)
{
	int ret;
	struct msg *sm;
	char sendto[NAMESIZE];
	char filename[100] = {0};
	FILE *fp = NULL;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	printf("输入文件传输对象:");
	scanf("%s", sendto);
	printf("输入文件名:");
	scanf("%s", filename);
	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		perror("fopen");
		free(sm);
		return;
	}
	sm->type = SENDFILE;
	strcpy(sm->object, sendto);
	strcpy(sm->data, "start\nsend from ");
	strcat(sm->data, name);
	sm->len = strlen(sm->data);
	if(send(sockfd, sm, sizeof(struct msg), 0) <= 0)
	{
		perror("send");
	}
	strcpy(sm->data, strrchr(filename, '/')+1);
	sm->len = strlen(sm->data);
	if(send(sockfd, sm, sizeof(struct msg), 0) <= 0)
	{
		perror("send");
	}

	while(1)
	{
		memset(sm->data, 0, sizeof(char) *BUFFSIZE);
		ret = fread(sm->data, sizeof(char), BUFFSIZE, fp);
		if(ferror(fp))
		{
			perror("fread");
			free(sm);
			return;
		}
		sm->len = ret;
		if(send(sockfd, sm, sizeof(struct msg), 0) <= 0)
		{
			perror("send");
		}
		usleep(10);
		if(feof(fp))
		{
			break;
		}
	}
	fclose(fp);
	memset(sm->data, 0, sizeof(char) *BUFFSIZE);
	strcpy(sm->data, "end");
	sm->len = strlen(sm->data);
	if(send(sockfd, sm, sizeof(struct msg), 0) <= 0)
	{
		perror("send");
	}
	free(sm);
	return;
}
void print_manage_menu()
{
	printf("***********菜单***********\n");
	printf("1.禁言		2.解禁\n");
	printf("3.退群		4.所有人状态\n");
	printf("5.退出\n");
}
void sendMsg_manage(int sockfd, char *name)
{
	int flg, key;
	char buff[100] = {0};
	struct msg *sm;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = MANAGE;
	print_manage_menu();
	scanf("%d", &flg);
	key = -1;
	switch(flg)
	{
	case 1:
		strcpy(sm->data, "jinyan");
		break;
	case 2:
		strcpy(sm->data, "jiejin");
		break;
	case 3:
		strcpy(sm->data, "tiren");
		break;
	case 4:
		strcpy(sm->data, "all");
		break;
	default :
		printf("input error.\n");
		return;
	}
	if(flg != 4)
	{
		printf("输入操作对象:");
		scanf("%s", sm->object);
	}
	if(send(sockfd, sm, sizeof(struct msg), 0) <= 0)
	{
		perror("send");
	}
	free(sm);
}
void sendMsg_online(int sockfd)
{
	struct msg *sm;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));

	sm->type = MANAGE;
	strcpy(sm->data, "online");
	sm->len = strlen(sm->data);
	if(send(sockfd, sm, sizeof(struct msg), 0) <= 0)
	{
		perror("send");
	}
	free(sm);
	return;
}
void sendMsg_group(int sockfd, char *name)
{
	struct msg *sm;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	char buff[BUFFSIZE-NAMESIZE-20];
	memset(sm, 0, sizeof(struct msg));
	printf("输入发送内容:");
	getchar();
	fgets(buff, sizeof(buff), stdin);
	buff[strlen(buff)-1] = 0;
	sprintf(sm->data, "[群聊]%s:%s", name, buff);
	sm->type = G_CHAT;
	sm->len = strlen(sm->data);
	strcpy(sm->object, "all");
	mySendMsg(sm, sockfd);
	free(sm);
	return;
}
void sendMsg_private(int sockfd, char *name)
{
	struct msg *sm;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	char buff[BUFFSIZE-NAMESIZE-20];
	memset(sm, 0, sizeof(struct msg));
	printf("输入发送内容:");
	getchar();
	fgets(buff, sizeof(buff), stdin);
	buff[strlen(buff)-1] = 0;
	sprintf(sm->data, "[私聊]%s:%s", name, buff);
	printf("输入发送对象:");
	scanf("%s", sm->object);
	sm->type = P_CHAT;
	sm->len = strlen(sm->data);
	mySendMsg(sm, sockfd);
	free(sm);
	return;
}
void print_menu()
{
	printf("************菜单************\n");
	printf("group		:发送消息至群聊\n");
	printf("private		:发送消息至私聊\n");
	printf("send_file	:发送文件\n");
	printf("online_users	:查询在线用户\n");
	printf("manage		:管理群成员\n");
	printf("help		:打印菜单\n");
	printf("exit		:退出登陆\n");
}
void *sendMsg(void *arg)
{
	int sockfd = ((struct myinform *)arg)->fd;
	char name[NAMESIZE];
	strcpy(name, ((struct myinform *)arg)->name);
	char buff[50] = {0};
	print_menu();
	scanf("%s", buff);
	while(strcmp(buff, "exit") != 0)
	{
		if(strcmp(buff, "group") == 0)
			sendMsg_group(sockfd, name);
		else if(strcmp(buff, "private") == 0)
			sendMsg_private(sockfd, name);
		else if(strcmp(buff, "send_file") == 0)
			sendMsg_file(sockfd, name);
		else if(strcmp(buff, "help") == 0)
			print_menu();
		else if(strcmp(buff, "online_users") == 0)
			sendMsg_online(sockfd);
		else if(strcmp(buff, "manage") == 0)
			sendMsg_manage(sockfd, name);
		else
			printf("输入错误。输入help查看菜单。\n");
		scanf("%s", buff);
	}
	return NULL;
}
void recvMsg_FILE(int sockfd)
{
	char filename[100];
	int recvlen;
	FILE *fp = NULL;
	struct msg *rm;
	rm = (struct msg *)malloc(sizeof(struct msg));
	recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
	}
	if(strlen(rm->data) >= 100)
	{
		printf("filename too long.\n");
		free(rm);
		return;
	}
	printf("正在接收文件...\n");
	strcpy(filename, rm->data);
	fp = fopen(filename, "w");
	if(fp == NULL)
	{
		perror("fopen");
		free(rm);
		return;
	}
	printf("文件名:%s\n", filename);
	while(1)
	{
		recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
		if(recvlen <= 0)
		{
			perror("recv");
			free(rm);
			return;
		}
		/*if(rm->type != SENDFILE)
		{
			printf("%s\n", rm->data);
			continue;
		}*/
		if(strncmp(rm->data, "end", 3) == 0)
		{
			break;
		}
		fwrite(rm->data, 1, rm->len, fp);
	}
	printf("接收成功。\n");
	fclose(fp);
	free(rm);
	return;
}
void *recvMsg(void *arg)
{
	int recvlen;
	int sockfd = *(int *)arg;
	struct msg *rm;
	rm = (struct msg *)malloc(sizeof(struct msg));
	printf("接收消息就绪.\n");
	while(1)
	{
		memset(rm, 0, sizeof(struct msg));
		recvlen = recv(sockfd, rm, sizeof(struct msg), 0);
		if(recv <= 0)
		{
			perror("recv");
		}
		else
		{
			if(rm->type == G_CHAT || rm->type == P_CHAT || rm->type == ACK)
			{
				if(strlen(rm->data) != 0)
				{
					printf("%s\n", rm->data);
				}
			}
			else if(rm->type == SENDFILE && strncmp(rm->data, "start", 5) == 0)
			{
				printf("准备接收文件....\n");
				printf("%s\n", rm->data);
				recvMsg_FILE(sockfd);
			}
			/*else
			{
				printf("wrong message.\n");
				printf("type:%d\tlen:%d\nobject=%sdata:%s\n",rm->type, rm->len, rm->object, rm->data);
			}*/
		}

	}
}
void main_menu(int sockfd)
{
	int flg = 0, ret;
	pthread_t tid1, tid2;
	char name[NAMESIZE] = {0};
	struct myinform m;
	print_main_menu();
	scanf("%d", &flg);
	while(flg != 4)
	{
		switch(flg)
		{
			case 1 : 
				login(sockfd, name);
				if(strlen(name) != 0)
				{
					strcpy(m.name, name);
					m.fd = sockfd;
					ret = pthread_create(&tid1, NULL, sendMsg, (void *)&m);
					if(ret != 0)
					{
						perror("pthread_create");
						exit(-1);
					}
					ret = pthread_create(&tid2, NULL, recvMsg, (void *)&sockfd);
					if(ret != 0)
					{
						perror("pthread_create");
						exit(-1);
					}
					ret = pthread_join(tid1, NULL);
					if(ret != 0)
					{
						perror("pthread_join");
						exit(-1);
					}
					ret = pthread_cancel(tid2);
					if(ret != 0)
					{
						perror("pthread_cancel");
						exit(-1);
					}
				}
				break;
			case 2 :
				reg(sockfd);
				break;
			case 3 :
				forgetpw(sockfd);
				break;
			default:
				printf("输入错误。\n");
				break;
		}
		print_main_menu();
		scanf("%d", &flg);
	}
	printf("exit.\n");
	return;
}
int main(int argc, char *argv[])
{
	int i, ret;
	int sockfd;
	char ip[20];
	struct sockaddr_in seraddr, cliaddr, ss;
	unsigned len = sizeof(struct sockaddr), recvlen;

	memset(&seraddr, 0, sizeof(seraddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(PORT);
	ret = inet_pton(AF_INET, "127.0.0.2", &seraddr.sin_addr);
	if(ret < 0)
	{
		perror("inet_pton error.");
	}
	Connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr));

	//seraddr.sin_addr.s_addr = htonl(argv[1]);
	getsockname(sockfd, (struct sockaddr *)&ss, &len);
	printf("local  address = %s:%d\n", inet_ntoa(ss.sin_addr), ntohs(ss.sin_port));
	getpeername(sockfd, (struct sockaddr *)&ss, &len);
	printf("server address = %s:%d\n", inet_ntop(AF_INET, &ss.sin_addr, ip, sizeof(ip)), ntohs(ss.sin_port));

	main_menu(sockfd);
}
