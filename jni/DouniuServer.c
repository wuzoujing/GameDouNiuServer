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


int listenfd,connfd[MAX_USERS];//分别记录服务器端的套接字与连接的多个客户端的套接字
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

//记录空闲的客户端的套接字描述符（-1为空闲）
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

//调用accept从listen接受的连接队列中取得一个连接
        connfd[i]=accept(listenfd,(struct sockaddr*)&cliaddr,&len);

        ticks=time(NULL);
        sprintf(buff,"%.24s\r\n",ctime(&ticks));
		strcpy(s_users[i].ipaddr, inet_ntoa(cliaddr.sin_addr));
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

//服务器关闭函数,若输入quit则退出服务器
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

//服务器接收并转发消息函数
//  每个client都对应一个接收线程，n为clientID
//  接收client的请求消息后，调用processMsg来处理消息，然后反馈消息给client
void receiveCMD(int n)
{
    char buff1[MAXLINE];
	int len;
    printf("[rcv_snd]thread rcv_snd[n:%d][id:%lu] created\n", n, pthread_self());

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

//消息处理关键函数
//  buffer为请求消息字符串，n为clientID
int processMsg(char* buffer, int n)
{
	char * data[DATA_LEN];
	char * str, *subtoken;
	char sendBuff[MAXLINE];
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

	//char info[256];
	char databuf[512];
	char resultStr[512];
	
	// 处理消息
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

			//反馈login OK消息给client
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);
			
			//把新用户的加入告知其他用户
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

			//反馈logout OK消息给client
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_LOGOUT, n, time(NULL), "logout ok!");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);

			//同时通知其他用户有人退出登录
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

			//清理变量及退出当前client的处理线程
			resetUserInfoAndConnfd(n);
			pthread_exit(0);
		}
		break;
	case CMD_PREPARE:
		{
			printf("[processMsg]CMD_PREPARE\n");

			//反馈prepare OK消息给client
			s_users[n].isPrepared = TRUE;
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_PREPARE, n, time(NULL), "prepare ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);

			//同时通知其他用户有人准备就绪
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

			//判断是否所有用户都准备就绪
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
			
			//若用户数超过2且所有用户都准备就绪，将洗牌并通知用户开始抢庄
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

				//通知用户开始抢庄
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
			
			//反馈TRYINGBANKER OK消息给client
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

			//判断是否所有用户都已抢庄或不抢
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

			//若所有用户都对抢庄或不抢，则决定庄家，并通知用户开始下注
			bankerIndex = judgeBanker();
			printf("[processMsg]all users is already tryingbanker, and banker is %d, will stake...\n",bankerIndex);
			
			//通知用户(除庄家外)开始下注
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

			//反馈STAKE OK消息给client
			printf("[processMsg]value:%d\n",atoi(data[OFT_DAT]));
			s_users[n].stake = atoi(data[OFT_DAT]);
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_STAKE, n, time(NULL), "stake ok");
			printf("[S->C][processMsg]%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			usleep(50);

			//同时通知其他用户其赌注
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

			//判断是否所有用户(除庄家)都已下注
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

			//若所有用户都已下注，则告知各用户自己的牌面
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

			//根据用户提交的是否有牛，计算牌型并反馈
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

			//通知其他用户其牌面及牌型
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

			//判断是否所有用户都已提交结果
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
			//若所有用户都已提交结果，则比较庄家和闲家，并结清赌注
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

			//通知庄家输赢结果
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

