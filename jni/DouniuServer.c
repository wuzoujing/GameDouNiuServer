#include "DouniuServer.h"

#ifdef USE_IN_ANDROID
#include <android/log.h>

#define TAG "[wzj][jni]DouniuServer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  , TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , TAG, __VA_ARGS__)
#define printf LOGV
#endif	//USE_IN_ANDROID


int listenfd,connfd[MAX_USERS];//�ֱ��¼�������˵��׽��������ӵĶ���ͻ��˵��׽���
UserInfo s_users[MAX_USERS];
Card s_cards[COUNT_CARDS];
int bankerIndex = 0;

void resetUserInfoAndConnfd(int i)
{
    connfd[i]=-1;
    s_users[i].id = -1;
    strcpy(s_users[i].name, "");
    strcpy(s_users[i].ipaddr, "");
	s_users[i].money = 10000;
    s_users[i].isPrepared = FALSE;
    s_users[i].bankerStatus = TBS_NONE;
    s_users[i].stake = 0;
    s_users[i].hasSubmitResult = FALSE;
}

void resetGameInfo(GameInfo* gameInfo)
{
	//gameInfo->cards;
	gameInfo->pokerPattern = POKER_PATTERN_WU_NIU;
	gameInfo->points = 0;
	gameInfo->maxCardValue = 0;
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

//��¼���еĿͻ��˵��׽�����������-1Ϊ���У�
    int i=0;
    for(i=0;i<MAX_USERS;i++)
    {
		resetUserInfoAndConnfd(i);
    }

    while(1)
    {
        len=sizeof(cliaddr);
        for(i=0;i<MAX_USERS;i++)
        {
            if(connfd[i]==-1)
            {
                break;
            }
        }

//����accept��listen���ܵ����Ӷ�����ȡ��һ������
        connfd[i]=accept(listenfd,(struct sockaddr*)&cliaddr,&len);

        ticks=time(NULL);
        sprintf(buff,"%.24s\r\n",ctime(&ticks));
		strcpy(s_users[i].ipaddr, inet_ntoa(cliaddr.sin_addr));
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

void appendAllUsersInfo(char* databuf)
{
	char info[256];
	int i = 0;
	for(i=0;i<MAX_USERS;i++)
	{
		if (s_users[i].id != -1)
		{
			sprintf(info, "%d#%s#%ld#", s_users[i].id, s_users[i].name, s_users[i].login_time);
			strcat(databuf, info);
		}
	}
	if (strlen(databuf) > 0)
	{
		databuf[strlen(databuf) - 1] = 0;
	}
	//return databuf;
}

int judgeBanker()
{
	int index = 0;
	srand(time(NULL));
	index = rand()%MAX_USERS;
	printf("[judgeBanker]index:%d\n",index);
	while(1)
	{
		if(connfd[index]!=-1 && s_users[index].bankerStatus == TBS_TRYING)
		{
			break;
		}
		else
		{
			index = (index+1)%MAX_USERS;
		}
	}
	printf("[judgeBanker]end index:%d\n",index);
	return index;
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
    printf("[quit]thread quit[id:%lu] created\n", pthread_self());
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
    printf("[rcv_snd]thread rcv_snd[n:%d][id:%lu] created\n", n, pthread_self());

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

//��Ϣ����ؼ�����
//  bufferΪ������Ϣ�ַ�����nΪclientID
int processMsg(char* buffer, int n)
{
	char * data[DATA_LEN];
	char * str, *subtoken;
	char sendBuff[MAXLINE];
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

	//char info[256];
	char databuf[512];
	char resultStr[512];
	
	// ������Ϣ
	switch(data[OFT_CMD][0])
	{
	case CMD_LIST:
		{
			printf("[processMsg]CMD_LIST\n");
			memset(databuf, 0, sizeof(databuf));
			appendAllUsersInfo(databuf);

			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_LIST, n, time(NULL), databuf);
			printf("[S->C][processMsg]%s\n",sendBuff);

			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);
		}
		break;
	case CMD_LOGIN:
		{
			printf("[processMsg]CMD_LOGIN\n");
			s_users[n].id = n;
			s_users[n].login_time = atol(data[OFT_TIM]);
			strcpy(s_users[n].name, data[OFT_FRM]);

			memset(databuf, 0, sizeof(databuf));
			appendAllUsersInfo(databuf);

			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_LOGIN, n, time(NULL), databuf);//data[OFT_FRM]
			printf("[S->C][processMsg]%s\n",sendBuff);

			//����login OK��Ϣ��client
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);
			
			//�����û��ļ����֪�����û�
			for(i=0;i<MAX_USERS;i++)
			{
				if(i!=n && connfd[i]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d", CMD_S2C_USER_IN, i, time(NULL), n);
					printf("[S->C][processMsg]notify login msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(50);
				}
				else if (i==n)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_users[n].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}
		}
		break;
	case CMD_LOGOUT:
		{
			printf("[processMsg]CMD_LOGOUT\n");

			//����logout OK��Ϣ��client
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_LOGOUT, n, time(NULL), "logout ok!");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);

			//ͬʱ֪ͨ�����û������˳���¼
			for(i=0;i<MAX_USERS;i++)
			{
				if(i!=n && connfd[i]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d", CMD_S2C_USER_OUT, i, time(NULL), n);
					printf("[S->C][processMsg]notify logout msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(50);
				}
				else if (i==n)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_users[n].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}

			printf("[processMsg]exit thread\n");
			close(connfd[n]);

			//����������˳���ǰclient�Ĵ����߳�
			resetUserInfoAndConnfd(n);
			pthread_exit(0);
		}
		break;
	case CMD_PREPARE:
		{
			printf("[processMsg]CMD_PREPARE\n");

			//����prepare OK��Ϣ��client
			s_users[n].isPrepared = TRUE;
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_PREPARE, n, time(NULL), "prepare ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);

			//ͬʱ֪ͨ�����û�����׼������
			for(i=0;i<MAX_USERS;i++)
			{
				if(i!=n && connfd[i]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d", CMD_S2C_USER_PREP, i, time(NULL), n);
					printf("[S->C][processMsg]notify prepare msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(50);
				}
				else if (i==n)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_users[n].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}

			//�ж��Ƿ������û���׼������
			int countUsers = 0;
			bool hasSomeoneNotPrepared = FALSE;//hasSomeoneNotPrepared = FALSE;
			for(i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1)
				{
					countUsers++;
					if (!s_users[i].isPrepared)
					{
						hasSomeoneNotPrepared = TRUE;
						printf("[processMsg]someone not prepare\n");
						break;
					}
				}
			}
			
			//���û�������2�������û���׼����������ϴ�Ʋ�֪ͨ�û���ʼ��ׯ
			printf("[processMsg]countUsers:%d,will fapai\n",countUsers);
			if (countUsers >= 2 && !hasSomeoneNotPrepared)
			{
				printf("[processMsg]all users is already preparing...will trying banker\n");
				printf("[processMsg]->initializePai\n");
				initializePai(s_cards, COUNT_CARDS);
				printf("[processMsg]->xiPai\n");
				xiPai(s_cards,COUNT_CARDS);
				printf("[processMsg]->faPai\n");
				faPai(s_users, MAX_USERS, s_cards, COUNT_CARDS);

				//֪ͨ�û���ʼ��ׯ
				for(i=0;i<MAX_USERS;i++)
				{
					if (s_users[i].id != -1)
					{
						memset(sendBuff, 0, sizeof(sendBuff));
						sprintf(sendBuff, "%c:%d:%ld:%s", CMD_S2C_WILL_BANKER, i, time(NULL), "WILL_BANKER");
						printf("[S->C][processMsg]notify msg[%s] to user[%d]\n",sendBuff, i);
						write(connfd[i],sendBuff,strlen(sendBuff));
						usleep(50);
					}
				}
				
				//TODO:send to client
				/*memset(databuf, 0, sizeof(databuf));
				for(i=0;i<MAX_USERS;i++)
				{
					if (s_users[i].id != -1)
					{
						sprintf(info, "%d#%s#%d#%d#%d#%d#%d#", s_users[i].id, s_users[i].name, s_users[i].gameInfo.cards[0].id, 
							s_users[i].gameInfo.cards[1].id, s_users[i].gameInfo.cards[2].id, s_users[i].gameInfo.cards[3].id, 
							s_users[i].gameInfo.cards[4].id);
						strcat(databuf, info);
					}
				}
				if (strlen(databuf) > 0)
				{
					databuf[strlen(databuf) - 1] = 0;
				}
				for (i=0;i<MAX_USERS;i++)
				{
					if (s_users[i].id != -1)
					{
						memset(sendBuff, 0, sizeof(sendBuff));
						sprintf(sendBuff, "%c:%d:%ld:%s", CMD_S2C_WILL_START, i, time(NULL), databuf);
						printf("[S->C][processMsg]%s\n",sendBuff);
						write(connfd[i],sendBuff,strlen(sendBuff));
					}
				}*/
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
				s_users[n].bankerStatus = TBS_TRYING;
				printf("[processMsg]TBS_TRYING\n");
			}
			else
			{
				s_users[n].bankerStatus = TBS_SKIP;
				printf("[processMsg]TBS_SKIP\n");
			}
			
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_TRYINGBANKER, n, time(NULL), "tryingbanker ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);

			//�ж��Ƿ������û�������ׯ����
			for(i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1)
				{
					if (s_users[i].bankerStatus == TBS_NONE)
					{
						printf("[processMsg]someone[%d] is TBS_NONE, wait ...\n", i);
						return;
					}
				}
			}

			//�������û�������ׯ�����������ׯ�ң���֪ͨ�û���ʼ��ע
			bankerIndex = judgeBanker();
			printf("[processMsg]all users is already tryingbanker, and banker is %d, will stake...\n",bankerIndex);
			
			//֪ͨ�û�(��ׯ����)��ʼ��ע
			for(i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1 && s_users[i].id != bankerIndex)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d", CMD_S2C_WILL_STAKE, i, time(NULL), bankerIndex);
					printf("[S->C][processMsg]notify msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(50);
				}
			}
		}
		break;
	case CMD_STAKE:
		{
			printf("[processMsg]CMD_STAKE\n");

			//����STAKE OK��Ϣ��client
			printf("[processMsg]value:%d\n",atoi(data[OFT_DAT]));
			s_users[n].stake = atoi(data[OFT_DAT]);
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_STAKE, n, time(NULL), "stake ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);

			//ͬʱ֪ͨ�����û����ע
			for(i=0;i<MAX_USERS;i++)
			{
				if(i!=n && connfd[i]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d#%s", CMD_S2C_STAKE_VALUE, i, time(NULL), n, data[OFT_DAT]);
					printf("[S->C][processMsg]notify stake msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(50);
				}
				else if (i==n)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_users[n].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}

			//�ж��Ƿ������û�(��ׯ��)������ע
			for(i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1 && s_users[i].id != bankerIndex)
				{
					if (s_users[i].stake <= 0)
					{
						printf("[processMsg]someone's stake is 0, wait ...\n");
						return;
					}
				}
			}

			//�������û�������ע�����֪���û��Լ�������
			for (i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1)
				{
					memset(databuf, 0, sizeof(databuf));
					sprintf(databuf, "%d#%s#%d#%d#%d#%d#%d", s_users[i].id, s_users[i].name, s_users[i].gameInfo.cards[0].id, 
						s_users[i].gameInfo.cards[1].id, s_users[i].gameInfo.cards[2].id, s_users[i].gameInfo.cards[3].id, 
						s_users[i].gameInfo.cards[4].id);
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%s", CMD_S2C_WILL_START, i, time(NULL), databuf);
					printf("[S->C][processMsg]msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(50);
				}
			}
		}
		break;
	case CMD_PLAY:
		{
			printf("[processMsg]CMD_PLAY\n");

			//�����û��ύ���Ƿ���ţ���������Ͳ�����
			printf("[processMsg]value:%d\n",atoi(data[OFT_DAT]));
			if (atoi(data[OFT_DAT]) == 1)
			{
				printf("[processMsg]youniu, need to calculate\n");
				caculateResult(&(s_users[n].gameInfo));
			}
			else
			{
				printf("[processMsg]wuniu, no need to calculate\n");
				resetGameInfo(&(s_users[n].gameInfo));
			}
			s_users[n].hasSubmitResult = TRUE;

			sprintf(sendBuff, "%c:%d:%ld:%d", CMD_PLAY, n, time(NULL), s_users[n].gameInfo.pokerPattern);
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);

			//֪ͨ�����û������漰����
			for(i=0;i<MAX_USERS;i++)
			{
				if(i!=n && connfd[i]!=-1)
				{
					memset(sendBuff, 0, sizeof(sendBuff));
					sprintf(sendBuff, "%c:%d:%ld:%d#%d", CMD_S2C_CARD_PATTERN, i, time(NULL), n, s_users[n].gameInfo.pokerPattern);
					printf("[S->C][processMsg]notify pattern msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(50);
				}
				else if (i==n)
				{
					printf("[processMsg]i:%d itself, ip:%s\n",i, s_users[n].ipaddr);
				}
				else
				{
					printf("[processMsg]i:%d connfd null\n",i);
				}
			}

			//�ж��Ƿ������û������ύ���
			for(i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1)
				{
					if (s_users[i].hasSubmitResult == FALSE)
					{
						printf("[processMsg]someone not submit result, wait ...\n");
						return;
					}
				}
			}
			//�������û������ύ�������Ƚ�ׯ�Һ��мң��������ע
			int resultForBanker = 0;
			int moneyWinOrLost = 0;
			for (i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1 && s_users[i].id != bankerIndex)
				{
					printf("i:%d->checkoutStake\n",i);
					memset(resultStr, 0, sizeof(resultStr));
					moneyWinOrLost = checkoutStake(&s_users[i], &s_users[bankerIndex], resultStr);
					resultForBanker -= moneyWinOrLost;
					printf("[processMsg]moneyWinOrLost:%d,resultForBanker:%d\n",moneyWinOrLost,resultForBanker);
					sprintf(sendBuff, "%c:%d:%ld:%d#%ld#%s", CMD_S2C_GAME_RESULT, i, time(NULL), moneyWinOrLost, s_users[n].money, resultStr);
					printf("[S->C][processMsg]notify result msg[%s] to user[%d]\n",sendBuff, i);
					write(connfd[i],sendBuff,strlen(sendBuff));
					usleep(50);
				}
			}

			//֪ͨׯ����Ӯ���
			if (resultForBanker>0)
			{
				strcpy(resultStr, "win");
			}
			else
			{
				strcpy(resultStr, "lost");
			}
			sprintf(sendBuff, "%c:%d:%ld:%d#%ld#%s", CMD_S2C_GAME_RESULT, i, time(NULL), resultForBanker, s_users[bankerIndex].money, resultStr);
			printf("[S->C][processMsg]notify result msg[%s] to banker[%d]\n",sendBuff, bankerIndex);
			write(connfd[bankerIndex],sendBuff,strlen(sendBuff));
			usleep(50);
			
			printf("[processMsg]play end\n");
		}
		break;
	default:
		return ERR;
	}

    return OK;
}

