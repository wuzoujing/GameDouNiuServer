#include "DouniuServer.h"
#include "sqliteHelper.h"

#ifdef USE_IN_ANDROID
#include <android/log.h>

#define TAG "[wzj][c]DouniuServer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  , TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , TAG, __VA_ARGS__)
#define printf LOGV
#endif	//USE_IN_ANDROID

//int connfd[MAX_USERS_EACH_ROOM];	//与服务器连接的多个客户端的套接字
//UserInfo s_users[MAX_USERS_EACH_ROOM];
//Card s_cards[COUNT_CARDS];
//int bankerIndex = 0;
int listenfd;						//服务器端的套接字
int connfd[MAX_CLIENTS_SUPPORT];	//与服务器连接的多个客户端的套接字
AllUserInfo s_arrayUser[MAX_CLIENTS_SUPPORT];
RoomInfo s_arrayRoom[COUNT_ROOMS];

#ifndef USE_IN_ANDROID
char sql[128];
sqlite3 *db;
#endif

//重置用户的游戏状态信息
void resetTempGameStatus(int rid,int uid)//void resetOneUserInfo(int rid,int uid)
{
	if (rid >= COUNT_ROOMS || uid >= MAX_USERS_EACH_ROOM)
	{
		printf("[resetTempGameStatus]rid or uid is out of range\n");
		return;
	}
    s_arrayRoom[rid].allGameInfo[uid].tempStatus.isPrepared = FALSE;
    s_arrayRoom[rid].allGameInfo[uid].tempStatus.bankerStatus = TBS_NONE;
    s_arrayRoom[rid].allGameInfo[uid].tempStatus.stake = 0;
    s_arrayRoom[rid].allGameInfo[uid].tempStatus.hasSubmitResult = FALSE;
}

//重置用户的游戏信息
void resetGameInfo(int rid, int uid)
{
	if (rid >= COUNT_ROOMS || uid >= MAX_USERS_EACH_ROOM)
	{
		printf("[resetGameInfo]rid or uid is out of range\n");
		return;
	}
	s_arrayRoom[rid].bankerIndex = -1;
	s_arrayRoom[rid].clientId[uid] = -1;

	//deskId
	s_arrayRoom[rid].allGameInfo[uid].deskId = -1;
	
	//tempStatus
	resetTempGameStatus(rid, uid);
}

//重置用户信息
void resetUserInfo(int clientID)
{
	if (clientID >= MAX_CLIENTS_SUPPORT)
	{
		printf("[resetUserInfo]clientID is out of range\n");
		return;
	}
		
	//s_arrayUser[clientID].deskId = -1;
	strcpy(s_arrayUser[clientID].ipaddr,"");

	//dbUserInfo
	strcpy(s_arrayUser[clientID].dbInfo.name,"");
	s_arrayUser[clientID].dbInfo.money = 0;
	strcpy(s_arrayUser[clientID].dbInfo.mark,"");
	s_arrayUser[clientID].dbInfo.avatarId = 0;
	s_arrayUser[clientID].dbInfo.level = 0;
}

//重置client相关的用户信息及套接字
void resetUserInfoAndConnfd(int clientID)
{
	connfd[clientID] = -1;
	resetUserInfo(clientID);
}

int initAndAccept()
{
	pthread_t thread;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    time_t ticks;
    char buff[MAXLINE];

//调用socket函数创建服务器端的套接字
    printf("Socket...\n");
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0)
    {
        printf("Socket created failed:%s\n",strerror(errno));
        return ERR;
    }

//调用bind函数使得服务器端的套接字与地址实现绑定
    printf("Bind...\n");
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        printf("Bind failed:%s\n",strerror(errno));
        return ERR;
    }

//调用listen函数，将一个主动连接套接字变为被动的倾听套接字
//在此过程中完成tcp的三次握手连接
    printf("listening...\n");
    listen(listenfd,LISTENQ);

//创建一个线程，对服务器程序进行管理（关闭）
    pthread_create(&thread,NULL,(void*)(&quit),NULL);

//初始化客户端的套接字描述符（-1为空闲）及相关变量
	int i = 0;
	for (i=0;i<MAX_CLIENTS_SUPPORT;i++)
	{
		resetUserInfoAndConnfd(i);
	}
    int rid=0;
	int uid=0;
	for(rid=0;rid<COUNT_ROOMS;rid++)
	{
		s_arrayRoom[rid].roomId = rid;
	    for(uid=0;uid<MAX_USERS_EACH_ROOM;uid++)
	    {
			resetGameInfo(rid, uid);
	    }
	}

#ifndef USE_IN_ANDROID
	//打开db，用于后续的用户检查
	int nResult = sqlite3_open(DB_NAME,&db);
    if (nResult != SQLITE_OK)
    {
		printf("[main]open sqlite3 failed:%s\n",sqlite3_errmsg(db));
        return ERR;
    }
    else
    {
        printf("[main]open sqlite3 success\n");
    }
#endif

    while(1)
    {
        len=sizeof(cliaddr);
        for(i=0;i<MAX_CLIENTS_SUPPORT;i++)
        {
            if(connfd[i]==-1)
            {
                break;
            }
        }
		if (i>=MAX_CLIENTS_SUPPORT)
		{
			printf("server is full, so sleep to wait other users leave...\n");
			usleep(SLEEP_TIME_LONG);
			continue;
		}

//调用accept从listen接受的连接队列中取得一个连接
        connfd[i] = accept(listenfd,(struct sockaddr*)&cliaddr,&len);

        ticks=time(NULL);
        sprintf(buff,"%.24s\r\n",ctime(&ticks));
		strcpy(s_arrayUser[i].ipaddr, inet_ntoa(cliaddr.sin_addr));
        printf("%s Connect from: %s,port %d\n\n",buff,inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

//针对当前套接字创建一个线程，对当前套接字的消息进行处理
        pthread_create(malloc(sizeof(pthread_t)),NULL,(void*)(&receiveCMD),(void*)i);
    }
	return OK;
}

void disconnect()
{
	printf("[disconnect]Byebye...\n");
	close(listenfd);
#ifndef USE_IN_ANDROID
    exit(0);
#endif	//USE_IN_ANDROID
}

void appendAllUsersInfo(int rid, char* databuf)
{
	char info[256];
	int i = 0;
	for(i=0;i<MAX_USERS_EACH_ROOM;i++)
	{
		if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
		{
			int clientID = s_arrayRoom[rid].clientId[i];
			//sprintf(info, "%d#%d#%s#%ld#", rid, s_arrayRoom[rid].allGameInfo[i].deskId, s_arrayUser[clientID].dbInfo.name, s_arrayUser[clientID].dbInfo.money);
			sprintf(info, "%d#%s#%ld#", s_arrayRoom[rid].allGameInfo[i].deskId, s_arrayUser[clientID].dbInfo.name, s_arrayUser[clientID].dbInfo.money);
			strcat(databuf, info);
		}
	}
	if (strlen(databuf) > 0)
	{
		databuf[strlen(databuf) - 1] = 0;
	}
}

void appendAllUsersInfoMore(int rid, char* databuf)
{
	char info[256];
	int i = 0;
	for(i=0;i<MAX_USERS_EACH_ROOM;i++)
	{
		if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
		{
			int clientID = s_arrayRoom[rid].clientId[i];
			sprintf(info, "%d#%s#%ld#%d#", s_arrayRoom[rid].allGameInfo[i].deskId, s_arrayUser[clientID].dbInfo.name, s_arrayUser[clientID].dbInfo.money, (s_arrayRoom[rid].allGameInfo[i].tempStatus.isPrepared?1:0) );
			strcat(databuf, info);
		}
	}
	if (strlen(databuf) > 0)
	{
		databuf[strlen(databuf) - 1] = 0;
	}
}

int judgeBanker(int rid)
{
	int index = 0;
	srand(time(NULL));
	index = rand()%MAX_USERS_EACH_ROOM;
	printf("[judgeBanker]index:%d\n",index);
	while(1)
	{
		int clientID = s_arrayRoom[rid].clientId[index];
		if(connfd[clientID]!=-1 && s_arrayRoom[rid].allGameInfo[index].tempStatus.bankerStatus == TBS_TRYING)
		{
			break;
		}
		else
		{
			index = (index+1)%MAX_USERS_EACH_ROOM;
		}
	}
	printf("[judgeBanker]end index:%d\n",index);
	return index;
}

int getDbUserInfo(char* name, DbUserInfo *dbInfo)
{
#ifndef USE_IN_ANDROID
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select * from user where name = '%s'", name);
	printf("[getDbUserInfo]sql:%s\n",sql);
	int nResult = select_by_table_with_name(db, sql, dbInfo);
	printf("[getDbUserInfo]name:%s, money:%ld\n",dbInfo->name, dbInfo->money);
	if (nResult == -1)
	{
		printf("[getDbUserInfo]");
		return ERR;
	}
	return OK;
#else
	// for test
	strcpy(dbInfo->name,name);
	dbInfo->money = 10000;
	dbInfo->level = 1;
	strcpy(dbInfo->mark,"my god");
	return OK;
#endif
}

int checkUserExist(char* name, DbUserInfo *dbInfo)
{
	return getDbUserInfo(name, dbInfo);
}

int main()
{
	if (initAndAccept() != OK)
	{
		printf("Occur some error, need exit.\n");
        return  -1;
	}
	printf("exit.\n");
    return 0;
}

//服务器关闭函数,若输入quit则退出服务器
void quit()
{
    char msg[10];
    printf("[quit]thread quit[deskId:%lu] created\n", pthread_self());
    while(1)
    {
        scanf("%s",msg);
        if(strcmp("quit",msg)==0)
        {
			disconnect();
        }
    }
}

//服务器接收并转发消息函数
//  每个client都对应一个接收线程，n为clientID
//  接收client的请求消息后，调用processMsg来处理消息，然后反馈消息给client
void receiveCMD(int n)
{
    char buff1[MAXLINE];
	int len;
    printf("[rcv_snd]thread rcv_snd[n:%d][deskId:%lu] created\n", n, pthread_self());

//接受当前用户的信息并将其转发给所有的用户
    while(1)
    {
        if((len=read(connfd[n],buff1,MAXLINE))>0)
        {
            buff1[len]=0;
			printf("\n[S<-C][rcv_snd]buff1: %s, n:%d\n", buff1, n);
			processMsg(buff1, n);
        }
    }
}

bool getEmptyDesk(int* rid, int* uid)
{
	int i;
	int j;
	for (i=0;i<COUNT_ROOMS;i++)
	{
		for (j=0;j<MAX_USERS_EACH_ROOM;j++)
		{
			if (s_arrayRoom[i].clientId[j] == -1)
			{
				*rid = i;
				*uid = j;
				return TRUE;
			}
		}
	}
	return FALSE;
}

bool getChairId(int n, int* rid, int* uid)
{
	int i;
	int j;
	for (i=0;i<COUNT_ROOMS;i++)
	{
		for (j=0;j<MAX_USERS_EACH_ROOM;j++)
		{
			if (s_arrayRoom[i].clientId[j] == n)
			{
				*rid = s_arrayRoom[i].roomId;
				*uid = s_arrayRoom[i].allGameInfo[j].deskId;
				return TRUE;
			}
		}
	}
	*rid=0;
	*uid=0;
	return FALSE;
}

//消息处理关键函数
//  buffer为请求消息字符串，n为clientID
int processMsg(char* buffer, int n)
{
	char * data[DATA_LEN];
	char * str, *subtoken;
	int i;
	memset(data, 0, sizeof(data));

	//按协议格式解析消息，然后存放到data数组
	for(str = buffer, i = 0; ; str = NULL, i++){
		subtoken = strtok(str, DATA_TOK);
		if(subtoken == NULL)
			break;
		data[i] = subtoken;
		printf("[processMsg]> data[%d] = %s\n", i, subtoken);
	}

	char databuf[512];
	char sendBuff[MAXLINE];

    // 根据clientId获得房间号rid及桌位号uid
	int rid = 0;
	int uid = 0;
	getChairId(n, &rid, &uid);
	printf("[processMsg]rid:%d,uid:%d\n",rid,uid);

	// 处理游戏相关消息(在游戏室)
	switch(data[OFT_CMD][0])
	{
	case CMD_DETAILS_INFO:
		{
			printf("[processMsg]CMD_DETAILS_INFO\n");
			//TODO
		}
		break;
	case CMD_LIST:
		{
			printf("[processMsg]CMD_LIST\n");
			memset(databuf, 0, sizeof(databuf));
			appendAllUsersInfo(rid, databuf);

			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_LIST, n, time(NULL), databuf);
			printf("[S->C][processMsg]%s\n",sendBuff);
 
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);
		}
		break;
	case CMD_LOGIN:
		{
			printf("[processMsg]CMD_LOGIN\n");
			DbUserInfo dbInfo;
			if (checkUserExist(data[OFT_FRM], &dbInfo) == ERR)
			{
#ifdef USE_IN_ANDROID
				//resetUserInfoAndConnfd(n);
				//TODO: 反馈账户不存在，提示注册
				printf("[processMsg]CMD_LOGIN temp user as no db\n");
				dbInfo.money = 11000;
				dbInfo.level = 3;
#else
				printf("[processMsg]CMD_LOGIN user is not exist, cannot connect\n");
				//反馈join room fail消息给client
				sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_LOGIN, n, time(NULL), "Fail");
				printf("[S->C][processMsg]%s\n",sendBuff);
				write(connfd[n],sendBuff,strlen(sendBuff));
				usleep(SLEEP_TIME_SHORT);
				break;
#endif
			}

			printf("[processMsg]CMD_LOGIN user exist\n");
			s_arrayUser[n].login_time = atol(data[OFT_TIM]);
			strcpy(s_arrayUser[n].dbInfo.name, data[OFT_FRM]);
			s_arrayUser[n].dbInfo.money = dbInfo.money;
			s_arrayUser[n].dbInfo.level = dbInfo.level;

			memset(databuf, 0, sizeof(databuf));
			sprintf(databuf, "%s#%ld#%d", s_arrayUser[n].dbInfo.name, s_arrayUser[n].dbInfo.money, s_arrayUser[n].dbInfo.level);

			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_LOGIN, n, time(NULL), databuf);
			printf("[S->C][processMsg]%s\n",sendBuff);

			//反馈login OK消息给client
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);
		}
		break;
	case CMD_LOGOUT:
		{
			printf("[processMsg]CMD_LOGOUT\n");

			//反馈logout OK消息给client
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_LOGOUT, n, time(NULL), "logout ok!");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			printf("[processMsg]exit thread\n");
			close(connfd[n]);

			//清理变量及退出当前client的处理线程
			resetGameInfo(rid, uid);
			resetUserInfoAndConnfd(n);
			pthread_exit(0);
		}
		break;
	case CMD_JOIN_ROOM:
		{
			printf("[processMsg]CMD_JOIN_ROOM\n");
			//DbUserInfo dbInfo;
			//getDbUserInfo(data[OFT_FRM], &dbInfo);
			
			if (getEmptyDesk(&rid, &uid) == TRUE)
			{
				printf("[processMsg]CMD_JOIN_ROOM rid:%d,uid:%d\n",rid, uid);
				s_arrayRoom[rid].roomId = rid;
				s_arrayRoom[rid].clientId[uid] = n;
				s_arrayRoom[rid].allGameInfo[uid].deskId = uid;
				
				memset(databuf, 0, sizeof(databuf));
				appendAllUsersInfoMore(rid, databuf);

				//反馈join room OK消息给client
				sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_JOIN_ROOM, uid, time(NULL), databuf);
				printf("[S->C][processMsg]%s\n",sendBuff);
				write(connfd[n],sendBuff,strlen(sendBuff));
				usleep(SLEEP_TIME_SHORT);
				
				//把新用户的加入告知其他用户
				memset(databuf, 0, sizeof(databuf));
				sprintf(databuf, "%d#%s#%ld#%d", s_arrayRoom[rid].allGameInfo[uid].deskId, s_arrayUser[n].dbInfo.name, s_arrayUser[n].dbInfo.money, (s_arrayRoom[rid].allGameInfo[uid].tempStatus.isPrepared?1:0) );
				for(i=0;i<MAX_USERS_EACH_ROOM;i++)
				{
					int clientID = s_arrayRoom[rid].clientId[i];
					printf("[processMsg]clientID:%d\n",clientID);
					if(i != uid && clientID != -1 && connfd[clientID] != -1)
					{
						memset(sendBuff, 0, sizeof(sendBuff));
						sprintf(sendBuff, "%c:%d:%ld:%s", CMD_S2C_USER_IN, i, time(NULL), databuf);
						printf("[S->C][processMsg]notify login msg[%s] to user[%d]\n",sendBuff, i);
						write(connfd[clientID],sendBuff,strlen(sendBuff));
						usleep(SLEEP_TIME_SHORT);
					}
					else if (i == uid)
					{
						printf("[processMsg]i:%d itself, ip:%s\n",i, s_arrayUser[clientID].ipaddr);
					}
					else
					{
						printf("[processMsg]i:%d deskId null or connfd null\n",i);
					}
				}
			}
			else
			{
				printf("[processMsg]CMD_JOIN_ROOM failed as no empty desk\n");
				//反馈join room fail消息给client
				sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_JOIN_ROOM, n, time(NULL), "Fail");
				printf("[S->C][processMsg]%s\n",sendBuff);
				write(connfd[n],sendBuff,strlen(sendBuff));
				usleep(SLEEP_TIME_SHORT);
			}
		}
		break;
	case CMD_EXIT_ROOM:
		{
			printf("[processMsg]CMD_EXIT_ROOM\n");
			//反馈exit room OK消息给client
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_EXIT_ROOM, n, time(NULL), "exit room ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			//同时通知其他用户有人退出游戏室
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				printf("[processMsg]clientID:%d\n",clientID);
				if(i != uid && clientID != -1 && connfd[clientID]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d\n", CMD_S2C_USER_OUT, i, time(NULL), uid);
					printf("[S->C][processMsg]notify logout msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[clientID],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
				else if (i == uid)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_arrayUser[clientID].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d deskId null or connfd null\n",i);
				}
			}

			//若不允许游戏中退出房间，清理变量如下；若允许游戏中退出房间，需要单独考虑
		 	resetGameInfo(rid, uid);
		}
		break;
	case CMD_PREPARE:
		{
			printf("[processMsg]CMD_PREPARE\n");

			//反馈prepare OK消息给client
			s_arrayRoom[rid].allGameInfo[uid].tempStatus.isPrepared = TRUE;
			//sprintf(sendBuff, "%c:%d:%ld:%s\nR:0:1473229652:1\nE:0:1473248562:WILL_BANKER\n", CMD_PREPARE, n, time(NULL), "prepare ok");
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_PREPARE, n, time(NULL), "prepare ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			//同时通知其他用户有人准备就绪
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if(i != uid && clientID != -1 && connfd[clientID] != -1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d\n", CMD_S2C_USER_PREP, i, time(NULL), uid);
					printf("[S->C][processMsg]notify prepare msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[clientID],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
				else if (i == uid)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_arrayUser[clientID].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}
			usleep(SLEEP_TIME_LONG);

			//判断是否所有用户都准备就绪
			int countUsers = 0;
			bool hasSomeoneNotPrepared = FALSE;//hasSomeoneNotPrepared = FALSE;
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
				{
					countUsers++;
					if (!s_arrayRoom[rid].allGameInfo[i].tempStatus.isPrepared)
					{
						hasSomeoneNotPrepared = TRUE;
						printf("[processMsg]someone not prepare\n");
						break;
					}
				}
			}
			
			//若用户数超过2且所有用户都准备就绪，将洗牌并通知用户开始抢庄
			printf("[processMsg]countUsers:%d\n",countUsers);
			if (countUsers >= 2 && !hasSomeoneNotPrepared)
			{
				printf("[processMsg]all users is already preparing...will trying banker(fapai in background)\n");
				printf("[processMsg]->initializePai\n");
				initializePai(s_arrayRoom[rid].cards, COUNT_CARDS);
				printf("[processMsg]->xiPai\n");
				xiPai(s_arrayRoom[rid].cards,COUNT_CARDS);
				printf("[processMsg]->faPai\n");
				faPai(s_arrayRoom[rid].allGameInfo, MAX_USERS_EACH_ROOM, s_arrayRoom[rid].cards, COUNT_CARDS);

				//通知用户开始抢庄
				for(i=0;i<MAX_USERS_EACH_ROOM;i++)
				{
					int clientID = s_arrayRoom[rid].clientId[i];
					if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
					{
						memset(sendBuff, 0, sizeof(sendBuff));
						sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_S2C_WILL_BANKER, i, time(NULL), "WILL_BANKER");
						printf("[S->C][processMsg]notify msg[%s] to user[%d]\n",sendBuff, i);
						write(connfd[clientID],sendBuff,strlen(sendBuff));
						usleep(SLEEP_TIME_SHORT);
					}
				}
			}
			else
			{
				printf("[processMsg]wait more users to prepare\n");
			}
		}
		break;
	case CMD_TRYINGBANKER:
		{
			printf("[processMsg]CMD_TRYINGBANKER\n");
			
			//反馈TRYINGBANKER OK消息给client
			printf("[processMsg]value:%d\n",atoi(data[OFT_DAT]));
			if (atoi(data[OFT_DAT]) == 1)
			{
				s_arrayRoom[rid].allGameInfo[uid].tempStatus.bankerStatus = TBS_TRYING;
				printf("[processMsg]TBS_TRYING\n");
			}
			else
			{
				s_arrayRoom[rid].allGameInfo[uid].tempStatus.bankerStatus = TBS_SKIP;
				printf("[processMsg]TBS_SKIP\n");
			}
			
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_TRYINGBANKER, n, time(NULL), "tryingbanker ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_NORMAL);

			//判断是否所有用户都已抢庄或不抢
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
				{
					if (s_arrayRoom[rid].allGameInfo[i].tempStatus.bankerStatus == TBS_NONE)
					{
						printf("[processMsg]someone[%d] is TBS_NONE, wait ...\n", i);
						return;
					}
				}
			}

			//若所有用户都对抢庄或不抢，则决定庄家，并通知用户开始下注
			s_arrayRoom[rid].bankerIndex = judgeBanker(rid);
			printf("[processMsg]all users is already tryingbanker, and banker is %d, will stake...\n",s_arrayRoom[rid].bankerIndex);
			
			//通知用户(除庄家外)开始下注
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)//if (s_users[i].deskId != -1 && s_users[i].deskId != bankerIndex)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d\n", CMD_S2C_WILL_STAKE, i, time(NULL), s_arrayRoom[rid].bankerIndex);
					printf("[S->C][processMsg]notify msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[clientID],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
			}
		}
		break;
	case CMD_STAKE:
		{
			printf("[processMsg]CMD_STAKE\n");

			//反馈STAKE OK消息给client
			printf("[processMsg]value:%d\n",atoi(data[OFT_DAT]));
			s_arrayRoom[rid].allGameInfo[uid].tempStatus.stake = atoi(data[OFT_DAT]);
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_STAKE, n, time(NULL), data[OFT_DAT]);
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			//同时通知其他用户其赌注
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if(i != uid && clientID != -1 && connfd[clientID]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d#%s\n", CMD_S2C_STAKE_VALUE, i, time(NULL), uid, data[OFT_DAT]);
					printf("[S->C][processMsg]notify stake msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[clientID],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
				else if (i == uid)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_arrayUser[clientID].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}
			usleep(SLEEP_TIME_LONG);

			//判断是否所有用户(除庄家)都已下注
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1 && s_arrayRoom[rid].allGameInfo[i].deskId != s_arrayRoom[rid].bankerIndex)
				{
					if (s_arrayRoom[rid].allGameInfo[i].tempStatus.stake <= 0)
					{
						printf("[processMsg]someone's stake is 0, wait ...\n");
						return;
					}
				}
			}

			//若所有用户都已下注，则告知各用户自己的牌面
			for (i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
				{
					memset(databuf, 0, sizeof(databuf));
					sprintf(databuf, "%d#%s#%d#%d#%d#%d#%d\n", s_arrayRoom[rid].allGameInfo[i].deskId, s_arrayUser[clientID].dbInfo.name, s_arrayRoom[rid].allGameInfo[i].gameInfo.cards[0].id, 
						s_arrayRoom[rid].allGameInfo[i].gameInfo.cards[1].id, s_arrayRoom[rid].allGameInfo[i].gameInfo.cards[2].id, s_arrayRoom[rid].allGameInfo[i].gameInfo.cards[3].id, 
						s_arrayRoom[rid].allGameInfo[i].gameInfo.cards[4].id);
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%s", CMD_S2C_WILL_SUBMIT, i, time(NULL), databuf);
					printf("[S->C][processMsg]msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[clientID],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
			}
		}
		break;
	case CMD_SUBMIT:
		{
			printf("[processMsg]CMD_SUBMIT\n");

			//根据用户提交的是否有牛，计算牌型并反馈
			printf("[processMsg]value:%d\n",atoi(data[OFT_DAT]));
			if (atoi(data[OFT_DAT]) == 1)
			{
				printf("[processMsg]youniu, need to calculate\n");
				calculateResult(&(s_arrayRoom[rid].allGameInfo[uid].gameInfo), TRUE);
			}
			else
			{
				printf("[processMsg]wuniu, no need to calculate\n");
				calculateResult(&(s_arrayRoom[rid].allGameInfo[uid].gameInfo), FALSE);
			}
			s_arrayRoom[rid].allGameInfo[uid].tempStatus.hasSubmitResult = TRUE;

			sprintf(sendBuff, "%c:%d:%ld:%d\n", CMD_SUBMIT, s_arrayRoom[rid].allGameInfo[uid].deskId, time(NULL), s_arrayRoom[rid].allGameInfo[uid].gameInfo.pokerPattern);
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			//通知其他用户其牌面及牌型
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if(i != uid && clientID != -1 && connfd[i]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d#%d#%d#%d#%d#%d#%d\n", CMD_S2C_CARD_PATTERN, i, time(NULL), n, s_arrayRoom[rid].allGameInfo[uid].gameInfo.pokerPattern, 
						s_arrayRoom[rid].allGameInfo[uid].gameInfo.cards[0].id, s_arrayRoom[rid].allGameInfo[uid].gameInfo.cards[1].id, s_arrayRoom[rid].allGameInfo[uid].gameInfo.cards[2].id, 
						s_arrayRoom[rid].allGameInfo[uid].gameInfo.cards[3].id, s_arrayRoom[rid].allGameInfo[uid].gameInfo.cards[4].id);
					printf("[S->C][processMsg]notify pattern msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[clientID],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
				else if (i == uid)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_arrayUser[clientID].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}
			usleep(SLEEP_TIME_LONG);

			//判断是否所有用户都已提交结果
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
				{
					if (s_arrayRoom[rid].allGameInfo[i].tempStatus.hasSubmitResult == FALSE)
					{
						printf("[processMsg]someone not submit result, wait ...\n");
						return;
					}
				}
			}
			//若所有用户都已提交结果，则比较庄家和闲家，并结清赌注
			int resultForBanker = 0;
			int moneyWinOrLost = 0;
			char info[512];
			char resultStr[512];
			int bankerIndex = s_arrayRoom[rid].bankerIndex;
			int bankerClientID = s_arrayRoom[rid].clientId[bankerIndex];
			memset(databuf, 0, sizeof(databuf));
			for (i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1 && s_arrayRoom[rid].allGameInfo[i].deskId != bankerIndex)
				{
					printf("i:%d->checkoutStake\n",i);
					memset(resultStr, 0, sizeof(resultStr));
					moneyWinOrLost = checkoutStake(&s_arrayUser[clientID].dbInfo, &s_arrayRoom[rid].allGameInfo[i], 
						&s_arrayUser[bankerClientID].dbInfo, &s_arrayRoom[rid].allGameInfo[bankerClientID], resultStr);
					resultForBanker -= moneyWinOrLost;
					printf("[processMsg]moneyWinOrLost:%d,resultForBanker:%d,resultStr:%s\n",moneyWinOrLost,resultForBanker,resultStr);
					memset(info, 0, sizeof(info));
					sprintf(info, "%d#%d#%ld#", s_arrayRoom[rid].allGameInfo[i].deskId, moneyWinOrLost, s_arrayUser[clientID].dbInfo.money);
					strcat(databuf, info);
				}
			}

			//通知各闲家和庄家所有的输赢结果
			if (resultForBanker>0)
			{
				strcpy(resultStr, "win");
			}
			else
			{
				strcpy(resultStr, "lost");
			}
			printf("[processMsg]resultForBanker:%d,resultStr:%s,banker money:%ld\n",resultForBanker,resultStr,s_arrayUser[bankerClientID].dbInfo.money);
			memset(info, 0, sizeof(info));
			sprintf(info, "%d#%d#%ld", s_arrayRoom[rid].allGameInfo[bankerIndex].deskId, resultForBanker, s_arrayUser[bankerClientID].dbInfo.money);
			strcat(databuf, info);
			for (i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_S2C_GAME_RESULT, i, time(NULL), databuf);
					printf("[S->C][processMsg]notify result msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[clientID],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
			}
			printf("[processMsg]submit end, will reset game\n");
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
    		{
				//resetGameInfo(rid, i);
				resetTempGameStatus(rid, i);
				s_arrayRoom[rid].bankerIndex = -1;
    		}
			printf("[processMsg]reset game end\n");
		}
		break;
	default:
		return ERR;
	}

    return OK;
}

