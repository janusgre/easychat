#include "head.h"
#include "/usr/include/mysql/mysql.h"
char *g_hostname = "localhost";
char *g_user = "root";
char *g_pw = "000000";
char *g_db_name = "chat";
int g_port = 3306;

int Query(MYSQL *conn, const char *sql);
MYSQL *Connectmysql();
int reg(int myfd, struct msg *prm);
int login(int myfd, struct msg *prm);
void forgetpw(int myfd, struct msg *prm);
int sendmsg_p(int myfd, struct msg *prm);
int sendmsg_g(int myfd, struct msg *prm);
void sendmsg_online(int myfd);
void manage_core(int myfd, char *name, int key);
void manage_all_users(int myfd);
void manage(int myfd, struct msg *prm);
void deal_msg(int myfd, struct msg *prm);
void* deal(void *argv);
void initUserState();

int Query(MYSQL *conn, const char *sql)
{
	if(mysql_real_query(conn, sql, strlen(sql)))
	{
		printf("query:%s:%s\n", sql, mysql_error(conn));
		exit(-1);
	}
	return 0;
}
MYSQL *Connectmysql()
{
	MYSQL *conn;
	conn = mysql_init(NULL);
	if(!mysql_real_connect(conn, g_hostname, g_user, g_pw, g_db_name, g_port, NULL, 0))
	{
		printf("mysql connect failed.\n");
		exit(-1);
	}
	return conn;
}
int reg(int myfd, struct msg *prm)
{
	int ret;
	char buff[100];
	char q[100], a[100], name[20], pw[20];
	MYSQL *conn = Connectmysql();
	MYSQL_RES *g_res;
	struct msg *sm = NULL;

	strcpy(name, prm->object);
	strcpy(pw, prm->data);
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;

	sprintf(buff, "select * from users where name='%s';", name);
	Query(conn, buff);
	g_res = mysql_store_result(conn);
	if(mysql_num_rows(g_res) != 0)
	{
		strcpy(sm->data, "reg failed");
		sm->len = strlen(sm->data);
		ret = send(myfd, sm, sizeof(struct msg), 0);		
		if(ret <= 0)
		{
			perror("reg_send");
		}
		free(sm);
		return -1;
	}
	strcpy(sm->data, "reg success");
	sm->len = strlen(sm->data);
	send(myfd, sm, sizeof(struct msg), 0);		

	sprintf(buff, "insert into users(name, pw, connfd, state) values('%s', '%s', -1, 0);", name, pw);
	Query(conn, buff);
	memset(prm, 0, sizeof(struct msg));
	int recvlen = recv(myfd, prm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
	}
	//sscanf(prm->data, "%s%s", q,a);
	char *pa = strchr(prm->data, '\n')+1;
	strcpy(a, pa);
	*pa = 0;
	strcpy(q, prm->data);
	q[strlen(q) -1 ] = 0;
	sprintf(buff, "update users set question='%s', answer='%s' where name ='%s' ;", q, a, name);
	Query(conn, buff);

	strcpy(sm->data, "set question success");
	sm->len = strlen(sm->data);
	send(myfd, sm, sizeof(struct msg), 0);		
	mysql_free_result(g_res);
	mysql_close(conn);
	return 0;
}

int login(int myfd, struct msg *prm)
{
	MYSQL *conn;
	struct msg *sm;
	char buff[100] = {0}, name[20], pw[20];
	sm = (struct msg*)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;
	conn = Connectmysql();
	strcpy(name, prm->object);
	strcpy(pw, prm->data);
	sprintf(buff, "select * from users where name='%s';", name);
	Query(conn, buff);
	MYSQL_RES *g_res = mysql_store_result(conn);
	if(mysql_num_rows(g_res) == 0)
	{
		strcpy(sm->data, "username no exist.");
		sm->len = strlen(sm->data);
		send(myfd, sm, sizeof(struct msg), 0);
		return 0;
	}
	printf("user %s try to login\n", prm->object);
	sprintf(buff, "select * from users where name='%s'and pw='%s';", name, pw);
	Query(conn, buff);
	g_res = mysql_store_result(conn);
	if(mysql_num_rows(g_res) != 0)
	{
		strcpy(sm->data, "login success.");
		sm->len = strlen(sm->data);
		send(myfd, sm, sizeof(struct msg), 0);
		sprintf(buff, "update users set connfd=%d where name='%s'", myfd, prm->object);
		Query(conn, buff);
		printf("user %s login success\n", prm->object);
		sprintf(buff, "select * from users where state=1;");//有没有管理员
		Query(conn, buff);
		g_res = mysql_store_result(conn);
		if(mysql_num_rows(g_res) == 0)//没有
		{
			sprintf(buff, "update users set state=1 where name='%s';",name);
			Query(conn, buff);
			strcpy(sm->data, "您已成为本群管理员.");
			sm->len = strlen(sm->data);
			send(myfd, sm, sizeof(struct msg), 0);
		}
		free(sm);
		return 1;
	}
	printf("user %s  wrong password\n", prm->object);
	strcpy(sm->data, "wrong password.");
	sm->len = strlen(sm->data);
	send(myfd, sm, sizeof(struct msg), 0);
	mysql_free_result(g_res);
	mysql_close(conn);
	free(sm);
	return 0;
}
void forgetpw(int myfd, struct msg *prm)
{
	MYSQL *conn;
	MYSQL_ROW g_row;
	struct msg *sm;
	char q[100], a[100], name[20], pw[20], buff[100];
	int recvlen, flg = 0;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;

	conn = Connectmysql();
	strcpy(name, prm->data);
	sprintf(buff, "select question,answer from users where name='%s';", name);
	Query(conn, buff);
	MYSQL_RES *g_res = mysql_store_result(conn);
	int i_Num_rows = mysql_num_rows(g_res);
	if(i_Num_rows == 0)
	{
		strcpy(sm->data, "user not exist.\n");
		send(myfd, sm, sizeof(struct msg), 0);
		free(sm);
		return;
	}
	printf("user %s try to reset password.\n", name);
	int iNum_fields = mysql_num_fields(g_res);
	while((g_row = mysql_fetch_row(g_res)))
	{
		strcpy(q, g_row[0]);
		strcpy(a, g_row[1]);
	}
	//printf("question:%s\nanswer:%s\n", q, a);
	strcpy(sm->object, "question");
	strcpy(sm->data, q);
	send(myfd, sm, sizeof(struct msg), 0);
	recvlen = recv(myfd, prm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
	}
	//printf("user's answer: %s\n", prm->data);
	flg = strcmp(a, prm->data);
	if(flg != 0)
	{
		strcpy(sm->data, "wrong");
		//printf("wrong.\n");
	}
	else
	{
		strcpy(sm->data, "correct");
	}
	send(myfd, sm, sizeof(struct msg), 0);
	if(flg != 0)
	{
		free(sm);
		return;
	}
	recvlen = recv(myfd, prm, sizeof(struct msg), 0);
	if(recvlen <= 0)
	{
		perror("recv");
	}
	strcpy(pw, prm->data);
	sprintf(buff, "update users set pw='%s' where name='%s';", pw, name);
	Query(conn, buff);
	
	strcpy(sm->data, "重设密码成功。");
	send(myfd, sm, sizeof(struct msg), 0);
	mysql_free_result(g_res);
	mysql_close(conn);
	free(sm);
	return;
}
int sendmsg_p(int myfd, struct msg *prm)
{
	char buff[100];
	struct msg *sm;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;
	MYSQL *conn = Connectmysql();
	MYSQL_RES *g_res;
	MYSQL_ROW g_row;
	int iNum_rows;
	sprintf(buff, "select * from users where connfd=%d and state=3;", myfd);
	Query(conn, buff);
	g_res = mysql_store_result(conn);
	iNum_rows = mysql_num_rows(g_res);
	if(iNum_rows != 0)
	{
		strcpy(sm->data, "您已被管理员禁言.");
		sm->len = strlen(sm->data);
		send(myfd, sm, sizeof(struct msg), 0);
		free(sm);
		return -1;
	}
	sprintf(buff, "select connfd from users where name='%s';", prm->object);
	Query(conn, buff);
	g_res = mysql_store_result(conn);
	iNum_rows = mysql_num_rows(g_res);
	if(iNum_rows == 0)		//不存在这个用户
	{
		strcpy(sm->data, "no such user.");
		sm->len = strlen(sm->data);
		if(send(myfd, sm, sizeof(struct msg), 0) <= 0)
		{
			perror("send");
		}
		free(sm);
		return -1;
	}
	int iNum_fields = mysql_num_fields(g_res);
	while((g_row = mysql_fetch_row(g_res)))
	{
		if(atoi(g_row[0]) == -1)
		{
			strcpy(sm->data, "user offline.");
			sm->len = strlen(sm->data);
			if(send(myfd, sm, sizeof(struct msg), 0) <= 0)
			{
				perror("send");
			}
			free(sm);
			return -1;
		}
		if(send(atoi(g_row[0]), prm, sizeof(struct msg), 0) <= 0)
		{
			perror("send");
		}
	}
	mysql_free_result(g_res);
	mysql_close(conn);
	free(sm);
	return 1;
}
int sendmsg_g(int myfd, struct msg *prm)
{
	char buff[100];
	MYSQL *conn = Connectmysql();
	MYSQL_RES *g_res;
	MYSQL_ROW g_row;
	struct msg *sm;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;	
	int iNum_rows;
	sprintf(buff, "select * from users where connfd=%d and state=3;", myfd);
	Query(conn, buff);
	g_res = mysql_store_result(conn);
	iNum_rows = mysql_num_rows(g_res);
	if(iNum_rows != 0)
	{
		strcpy(sm->data, "您已被管理员禁言.");
		sm->len = strlen(sm->data);
		send(myfd, sm, sizeof(struct msg), 0);
		free(sm);
		return -1;
	}
	sprintf(buff, "select * from users where connfd=%d and state=2;", myfd);
	Query(conn, buff);
	g_res = mysql_store_result(conn);
	iNum_rows = mysql_num_rows(g_res);
	if(iNum_rows != 0)
	{
		strcpy(sm->data, "您已被管理员踢出本群.");
		sm->len = strlen(sm->data);
		send(myfd, sm, sizeof(struct msg), 0);
		free(sm);
		mysql_free_result(g_res);
		mysql_close(conn);
		return -1;
	}
	sprintf(buff, "select connfd from users where connfd!=-1;");
	Query(conn, buff);
	g_res = mysql_store_result(conn);
	iNum_rows = mysql_num_rows(g_res);
	int iNum_fields = mysql_num_fields(g_res);
	while((g_row = mysql_fetch_row(g_res)))
	{
		if(send(atoi(g_row[0]), prm, sizeof(struct msg), 0) <= 0)
		{
			perror("send");
		}
	}
	mysql_free_result(g_res);
	free(sm);
	mysql_close(conn);
	return 1;
}
void sendmsg_online(int myfd)
{
	struct msg *sm;
	char buff[100];
	MYSQL *conn = Connectmysql();
	MYSQL_RES *g_res;
	MYSQL_ROW g_row;
	int iNum_rows;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;
	strcat(sm->data, "online users list:\n");
	sprintf(buff, "select name from users where connfd!=-1;");
	Query(conn, buff);
	g_res = mysql_store_result(conn);
	iNum_rows = mysql_num_rows(g_res);
	int iNum_fields = mysql_num_fields(g_res);
	while((g_row = mysql_fetch_row(g_res)))
	{
		strcat(sm->data, g_row[0]);
		strcat(sm->data, "\t");
	}
	sm->len = strlen(sm->data);
	if(send(myfd, sm, sizeof(struct msg), 0) <= 0)
	{
		perror("send");
	}
	mysql_free_result(g_res);
	free(sm);
	mysql_close(conn);
	return;
}
void manage_core(int myfd, char *name, int key)
{
	int newfd;
	MYSQL *conn;
	char buff[100] = {0};
	struct msg *sm;
	MYSQL_ROW g_row;
	sm = (struct msg*)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;
	conn = Connectmysql();
	sprintf(buff, "select connfd from users where name='%s';", name);
	Query(conn, buff);
	MYSQL_RES *g_res = mysql_store_result(conn);
	if(mysql_num_rows(g_res) == 0)
	{
		strcpy(sm->data, "username no exist.");
		sm->len = strlen(sm->data);
		send(myfd, sm, sizeof(struct msg), 0);
		free(sm);
		return;
	}
	while((g_row = mysql_fetch_row(g_res)))
	{
		newfd = atoi(g_row[0]);
		if(newfd != -1)
		{
			memset(sm->data, 0, BUFFSIZE);
			strcpy(sm->data, "您已被管理员");
			if(key == 0)
			{
				strcat(sm->data, "恢复普通群成员身份.");
			}
			else if(key == 2)
			{
				strcat(sm->data, "踢出群聊.");
			}
			else if(key == 3)
			{
				strcat(sm->data, "禁言.");
			}
			if(send(newfd, sm, sizeof(struct msg), 0) <= 0)
			{
				perror("send");
			}
		}
	}
	sprintf(buff, "update users set state=%d where name='%s';", key, name);
	Query(conn, buff);

	strcpy(sm->data, "操作成功");
	sm->len = strlen(sm->data);
	send(myfd, sm, sizeof(struct msg), 0);
	free(sm);
	mysql_free_result(g_res);
	mysql_close(conn);
	return;
}
void manage_all_users(int myfd)
{
	struct msg *sm;
	sm = (struct msg*)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;
	MYSQL *conn;
	MYSQL_ROW g_row;
	char buff[100];
	conn = Connectmysql();
	sprintf(buff, "select name,state from chat.users;");
	Query(conn, buff);
	MYSQL_RES *g_res = mysql_store_result(conn);
	int i_Num_rows = mysql_num_rows(g_res);
	int iNum_fields = mysql_num_fields(g_res);
	while((g_row = mysql_fetch_row(g_res)))
	{
		strcat(sm->data, g_row[0]);
		strcat(sm->data, "\t");
		if(atoi(g_row[1]) == 0)
		{
			strcat(sm->data, "普通成员\n");
		}
		else if(atoi(g_row[1]) == 1)
		{
			strcat(sm->data, "管理员\n");
		}
		else if(atoi(g_row[1]) == 2)
		{
			strcat(sm->data, "已被踢群\n");
		}
		else if(atoi(g_row[1]) == 3)
		{
			strcat(sm->data, "已被禁言\n");
		}
	}

	send(myfd, sm, sizeof(struct msg), 0);
	free(sm);
	mysql_free_result(g_res);
	mysql_close(conn);
	return;
}
void manage(int myfd, struct msg *prm)
{
	int key = -1;
	if(strncmp(prm->data, "online", 6) == 0)
	{
		sendmsg_online(myfd);
		return;
	}
	MYSQL *conn;
	char buff[100] = {0};
	struct msg *sm;
	sm = (struct msg*)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;
	conn = Connectmysql();
	sprintf(buff, "select * from users where connfd=%d and state=1;", myfd);
	Query(conn, buff);
	MYSQL_RES *g_res = mysql_store_result(conn);
	if(mysql_num_rows(g_res) == 0)
	{
		strcpy(sm->data, "您没有管理员权限.");
		sm->len = strlen(sm->data);
		send(myfd, sm, sizeof(struct msg), 0);
		free(sm);
		mysql_free_result(g_res);
		mysql_close(conn);
		return;
	}
	if(strncmp(prm->data, "jinyan", 6) == 0)
	{
		key = 3;
		manage_core(myfd, prm->object, key);
	}
	else if(strncmp(prm->data, "jiejin", 6) == 0)
	{
		key = 0;
		manage_core(myfd, prm->object, key);
	}
	else if(strncmp(prm->data, "tiren", 5) == 0)
	{
		key = 2;
		manage_core(myfd, prm->object, key);
	}
	else if(strncmp(prm->data, "all", 3) == 0)
	{
		manage_all_users(myfd);
	}
	free(sm);
	mysql_free_result(g_res);
	mysql_close(conn);
	return;
}
void deal_msg(int myfd, struct msg *prm)
{
	int ret = 0;
	struct msg *sm;
	sm = (struct msg *)malloc(sizeof(struct msg));
	memset(sm, 0, sizeof(struct msg));
	sm->type = ACK;
	switch(prm->type)
	{
		case MANAGE :
			manage(myfd, prm);
			break;
		case G_CHAT :
			ret = sendmsg_g(myfd, prm);
			break;
		case P_CHAT :
			ret = sendmsg_p(myfd, prm);
			break;
		case SENDFILE :
			//sendfile(myfd, prm);
			sendmsg_p(myfd, prm);
			if(strcmp(prm->data, "end") == 0)
			{
				ret = 1;
			}
			break;
	}
	if(ret == 1)
	{
		strcpy(sm->data, "send success.");
		sm->len = strlen(sm->data);
		ret = send(myfd, sm, sizeof(struct msg), 0);
		if(ret <= 0)
		{
			perror("send");
		}
	}
	free(sm);
	return;
}
void* deal(void *argv)
{
	struct msg *psm, *prm;
	int ret, recvlen = 0;
	int myfd = *(int *)argv;

	psm = prm = NULL;
	psm = (struct msg*)malloc(sizeof(struct msg));
	prm = (struct msg*)malloc(sizeof(struct msg));

	memset(psm, 0, sizeof(struct msg));
	psm->type = ACK;

	while(1)
	{
		memset(prm, 0, sizeof(struct msg));
		recvlen = recv(myfd, (void *)prm, sizeof(struct msg), 0);
		if(0 == recvlen)
		{
			printf("one user network interruption\n");
			return NULL;
		}
		//printf("*********************\n");
		//printf("connfd:%d\ttype:%d\tlen:%d\n", myfd, prm->type, prm->len);
		//printf("object:%s\ndata:%s\n", prm->object, prm->data);
		if(prm->type > 2)
		{
			send(myfd, (void *)psm, sizeof(struct msg), 0);
		}

		if(prm->type == REG)		//注册
		{
			ret = reg(myfd, prm);
			continue;
		}

		if(prm->type == LOGIN)		//登陆
		{
			login(myfd, prm);
			continue;
		}

		if(prm->type == FORGETPW)	//忘记密码
		{
			forgetpw(myfd, prm);
		}
		if(prm->type == EXIT)		//退出
		{
			printf("user %s offline\n", prm->object);
			free(psm);
			free(prm);
			close(myfd);
			return NULL;
		}
		if(prm->type > 2 && prm->type <= 6)	//其他
		{
			deal_msg(myfd, prm);
		}
	}
}
void initUserState()
{
	MYSQL *conn;
	conn = mysql_init(NULL);
	if(!mysql_real_connect(conn, g_hostname, g_user, g_pw, g_db_name, g_port, NULL, 0))
	{
		printf("mysql connect failed.\n");
		exit(-1);
	}
	printf("connect mysql success\n");
	Query(conn, "use chat;");
	Query(conn, "update users set connfd=-1;");
	Query(conn, "delete from chat.users where name='';");
	mysql_close(conn);
}
int main()
{
	int cliaddrlen, ret;
	int sockfd, newfd;
	struct sockaddr_in seraddr, cliaddr;
	pthread_t tid;

	initUserState();

	memset(&seraddr, 0, sizeof(seraddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(PORT);
	seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddrlen = sizeof(cliaddr);

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	Bind(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
	Listen(sockfd, BACKLOG);

	while(1)
	{
		newfd = Accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
		ret = pthread_create(&tid, NULL, deal, (void *)&newfd);
		if(ret != 0)
		{
			perror("pthread_create");
			exit(-1);
		}
	}
	return 0;
}
