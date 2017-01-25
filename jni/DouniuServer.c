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

//int connfd[MAX_USERS_EACH_ROOM];	//����������ӵĶ���ͻ��˵��׽���
//UserInfo s_users[MAX_USERS_EACH_ROOM];
//Card s_cards[COUNT_CARDS];
//int bankerIndex = 0;
int listenfd;						//�������˵��׽���
int connfd[MAX_CLIENTS_SUPPORT];	//����������ӵĶ���ͻ��˵��׽���
AllUserInfo s_arrayUser[MAX_CLIENTS_SUPPORT];
RoomInfo s_arrayRoom[COUNT_ROOMS];

#ifndef USE_IN_ANDROID
char sql[128];
sqlite3 *db;
#endif

//�����û�����Ϸ״̬��Ϣ
void resetTempGameStatus(int rid,int uid)//void resetOneUserInfo(int rid,int uid)
{
    s_arrayRoom[rid].allGameInfo[uid].tempStatus.isPrepared = FALSE;
    s_arrayRoom[rid].allGameInfo[uid].tempStatus.bankerStatus = TBS_NONE;
    s_arrayRoom[rid].allGameInfo[uid].tempStatus.stake = 0;
    s_arrayRoom[rid].allGameInfo[uid].tempStatus.hasSubmitResult = FALSE;
}

//�����û�����Ϸ��Ϣ
void resetGameInfo(int rid, int uid)
{
	s_arrayRoom[rid].bankerIndex = -1;
	s_arrayRoom[rid].clientId[uid] = -1;

	//deskId
	s_arrayRoom[rid].allGameInfo[uid].deskId = -1;
	
	//tempStatus
	resetTempGameStatus(rid, uid);
}

//�����û���Ϣ
void resetUserInfo(int clientID)
{
	//s_arrayUser[clientID].deskId = -1;
	strcpy(s_arrayUser[clientID].ipaddr,"");

	//dbUserInfo
	strcpy(s_arrayUser[clientID].dbInfo.name,"");
	s_arrayUser[clientID].dbInfo.money = 0;
	strcpy(s_arrayUser[clientID].dbInfo.mark,"");
	s_arrayUser[clientID].dbInfo.avatarId = 0;
	s_arrayUser[clientID].dbInfo.level = 0;
}

//����client��ص��û���Ϣ���׽���
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

//����socket���������������˵��׽���
    printf("Socket...\n");
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0)
    {
        printf("Socket created failed:%s\n",strerror(errno));
        return ERR;
    }

//����bind����ʹ�÷������˵��׽������ַʵ�ְ�
    printf("Bind...\n");
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        printf("Bind failed:%s\n",strerror(errno));
        return ERR;
    }

//����listen��������һ�����������׽��ֱ�Ϊ�����������׽���
//�ڴ˹��������tcp��������������
    printf("listening...\n");
    listen(listenfd,LISTENQ);

//����һ���̣߳��Է�����������й����رգ�
    pthread_create(&thread,NULL,(void*)(&quit),NULL);

//��ʼ���ͻ��˵��׽�����������-1Ϊ���У�����ر���
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
	//��db�����ں������û����
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

//����accept��listen���ܵ����Ӷ�����ȡ��һ������
        connfd[i] = accept(listenfd,(struct sockaddr*)&cliaddr,&len);

        ticks=time(NULL);
        sprintf(buff,"%.24s\r\n",ctime(&ticks));
		strcpy(s_arrayUser[i].ipaddr, inet_ntoa(cliaddr.sin_addr));
        printf("%s Connect from: %s,port %d\n\n",buff,inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

//��Ե�ǰ�׽��ִ���һ���̣߳��Ե�ǰ�׽��ֵ���Ϣ���д���
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

//�������رպ���,������quit���˳�������
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

//���������ղ�ת����Ϣ����
//  ÿ��client����Ӧһ�������̣߳�nΪclientID
//  ����client��������Ϣ�󣬵���processMsg��������Ϣ��Ȼ������Ϣ��client
void receiveCMD(int n)
{
    char buff1[MAXLINE];
	int len;
    printf("[rcv_snd]thread rcv_snd[n:%d][deskId:%lu] created\n", n, pthread_self());

//���ܵ�ǰ�û�����Ϣ������ת�������е��û�
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

//��Ϣ����ؼ�����
//  bufferΪ������Ϣ�ַ�����nΪclientID
int processMsg(char* buffer, int n)
{
	char * data[DATA_LEN];
	char * str, *subtoken;
	int i;
	memset(data, 0, sizeof(data));

	//��Э���ʽ������Ϣ��Ȼ���ŵ�data����
	for(str = buffer, i = 0; ; str = NULL, i++){
		subtoken = strtok(str, DATA_TOK);
		if(subtoken == NULL)
			break;
		data[i] = subtoken;
		printf("[processMsg]> data[%d] = %s\n", i, subtoken);
	}

	char databuf[512];
	char sendBuff[MAXLINE];

    // ����clientId��÷����rid����λ��uid
	int rid = 0;
	int uid = 0;
	getChairId(n, &rid, &uid);
	printf("[processMsg]rid:%d,uid:%d\n",rid,uid);

	// ������Ϸ�����Ϣ(����Ϸ��)
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
				//TODO: �����˻������ڣ���ʾע��
				printf("[processMsg]CMD_LOGIN temp user as no db\n");
				dbInfo.money = 11000;
				dbInfo.level = 3;
#else
				printf("[processMsg]CMD_LOGIN user is not exist, cannot connect\n");
				//����join room fail��Ϣ��client
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

			//����login OK��Ϣ��client
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			/*memset(databuf, 0, sizeof(databuf));
			appendAllUsersInfo(databuf);

			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_LOGIN, n, time(NULL), databuf);
			printf("[S->C][processMsg]%s\n",sendBuff);

			//����login OK��Ϣ��client
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);
			
			//�����û��ļ����֪�����û�
			memset(databuf, 0, sizeof(databuf));
			sprintf(databuf, "%d#%s#%ld", s_users[n].deskId, s_users[n].name, s_users[n].money);
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				if(i!=n && connfd[i]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%s", CMD_S2C_USER_IN, i, time(NULL), databuf);
					printf("[S->C][processMsg]notify login msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
				else if (i==n)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_users[n].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}*/
		}
		break;
	case CMD_LOGOUT:
		{
			printf("[processMsg]CMD_LOGOUT\n");

			//����logout OK��Ϣ��client
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_LOGOUT, n, time(NULL), "logout ok!");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			/*//ͬʱ֪ͨ�����û������˳���¼
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				if(i!=n && connfd[i]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d\n", CMD_S2C_USER_OUT, i, time(NULL), n);
					printf("[S->C][processMsg]notify logout msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
				else if (i==n)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_users[n].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}*/

			printf("[processMsg]exit thread\n");
			close(connfd[n]);

			//����������˳���ǰclient�Ĵ����߳�
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
				appendAllUsersInfo(rid, databuf);

				//����join room OK��Ϣ��client
				sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_JOIN_ROOM, n, time(NULL), databuf);
				printf("[S->C][processMsg]%s\n",sendBuff);
				write(connfd[n],sendBuff,strlen(sendBuff));
				usleep(SLEEP_TIME_SHORT);
				
				//�����û��ļ����֪�����û�
				memset(databuf, 0, sizeof(databuf));
				sprintf(databuf, "%d#%s#%ld", s_arrayRoom[rid].allGameInfo[uid].deskId, s_arrayUser[n].dbInfo.name, s_arrayUser[n].dbInfo.money);
				for(i=0;i<MAX_USERS_EACH_ROOM;i++)
				{
					int clientID = s_arrayRoom[rid].clientId[i];
					printf("[processMsg]clientID:%d\n",clientID);
					if(i != uid && clientID != -1 && connfd[clientID] != -1)
					{
						memset(sendBuff, 0, sizeof(sendBuff));
						sprintf(sendBuff, "%c:%d:%ld:%s", CMD_S2C_USER_IN, i, time(NULL), databuf);
						printf("[S->C][processMsg]notify login msg[%s] to user[%d]\n",sendBuff, i);
						write(connfd[i],sendBuff,strlen(sendBuff));
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
				//����join room fail��Ϣ��client
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
			//����exit room OK��Ϣ��client
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_EXIT_ROOM, n, time(NULL), "exit room ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			//ͬʱ֪ͨ�����û������˳���Ϸ��
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				printf("[processMsg]clientID:%d\n",clientID);
				if(i != uid && clientID != -1 && connfd[clientID]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d\n", CMD_S2C_USER_OUT, i, time(NULL), uid);
					printf("[S->C][processMsg]notify logout msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
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

			//����������Ϸ���˳����䣬����������£���������Ϸ���˳����䣬��Ҫ��������
		 	resetGameInfo(rid, uid);
		}
		break;
	case CMD_PREPARE:
		{
			printf("[processMsg]CMD_PREPARE\n");

			//����prepare OK��Ϣ��client
			s_arrayRoom[rid].allGameInfo[uid].tempStatus.isPrepared = TRUE;
			//sprintf(sendBuff, "%c:%d:%ld:%s\nR:0:1473229652:1\nE:0:1473248562:WILL_BANKER\n", CMD_PREPARE, n, time(NULL), "prepare ok");
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_PREPARE, n, time(NULL), "prepare ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			//ͬʱ֪ͨ�����û�����׼������
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if(i != uid && clientID != -1 && connfd[clientID] != -1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d\n", CMD_S2C_USER_PREP, i, time(NULL), n);
					printf("[S->C][processMsg]notify prepare msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
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

			//�ж��Ƿ������û���׼������
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
			
			//���û�������2�������û���׼����������ϴ�Ʋ�֪ͨ�û���ʼ��ׯ
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

				//֪ͨ�û���ʼ��ׯ
				for(i=0;i<MAX_USERS_EACH_ROOM;i++)
				{
					if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
					{
						memset(sendBuff, 0, sizeof(sendBuff));
						sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_S2C_WILL_BANKER, i, time(NULL), "WILL_BANKER");
						printf("[S->C][processMsg]notify msg[%s] to user[%d]\n",sendBuff, i);
						write(connfd[i],sendBuff,strlen(sendBuff));
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
			
			//����TRYINGBANKER OK��Ϣ��client
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

			//�ж��Ƿ������û�������ׯ����
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

			//�������û�������ׯ�����������ׯ�ң���֪ͨ�û���ʼ��ע
			s_arrayRoom[rid].bankerIndex = judgeBanker(rid);
			printf("[processMsg]all users is already tryingbanker, and banker is %d, will stake...\n",s_arrayRoom[rid].bankerIndex);
			
			//֪ͨ�û�(��ׯ����)��ʼ��ע
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)//if (s_users[i].deskId != -1 && s_users[i].deskId != bankerIndex)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d\n", CMD_S2C_WILL_STAKE, i, time(NULL), s_arrayRoom[rid].bankerIndex);
					printf("[S->C][processMsg]notify msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
			}
		}
		break;
	case CMD_STAKE:
		{
			printf("[processMsg]CMD_STAKE\n");

			//����STAKE OK��Ϣ��client
			printf("[processMsg]value:%d\n",atoi(data[OFT_DAT]));
			s_arrayRoom[rid].allGameInfo[uid].tempStatus.stake = atoi(data[OFT_DAT]);
			sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_STAKE, n, time(NULL), data[OFT_DAT]);
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(SLEEP_TIME_SHORT);

			//ͬʱ֪ͨ�����û����ע
			for(i=0;i<MAX_USERS_EACH_ROOM;i++)
			{
				int clientID = s_arrayRoom[rid].clientId[i];
				if(i != uid && clientID != -1 && connfd[clientID]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d#%s\n", CMD_S2C_STAKE_VALUE, i, time(NULL), n, data[OFT_DAT]);
					printf("[S->C][processMsg]notify stake msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
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

			//�ж��Ƿ������û�(��ׯ��)������ע
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

			//�������û�������ע�����֪���û��Լ�������
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
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(SLEEP_TIME_SHORT);
				}
			}
		}
		break;
	case CMD_SUBMIT:
		{
			printf("[processMsg]CMD_SUBMIT\n");

			//�����û��ύ���Ƿ���ţ���������Ͳ�����
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

			//֪ͨ�����û������漰����
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
					write(connfd[i],sendBuff,strlen(sendBuff));
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

			//�ж��Ƿ������û������ύ���
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
			//�������û������ύ�������Ƚ�ׯ�Һ��мң��������ע
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

			//֪ͨ���мҺ�ׯ�����е���Ӯ���
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
				if (s_arrayRoom[rid].allGameInfo[i].deskId != -1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%s\n", CMD_S2C_GAME_RESULT, i, time(NULL), databuf);
					printf("[S->C][processMsg]notify result msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
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

