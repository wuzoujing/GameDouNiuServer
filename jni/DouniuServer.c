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
        connfd[i]=-1;
		s_users[i].id = -1;
		strcpy(s_users[i].name, "");
		strcpy(s_users[i].ipaddr, "");
		s_users[i].isPrepared = FALSE;
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

	char info[256];
	char buf[256];
	bool hasSomeoneNotPrepared = TRUE;
	int countUsers = 0;
	
	// ������Ϣ
	switch(data[OFT_CMD][0])
	{
	case CMD_LIST:
		{
			printf("[processMsg]CMD_LIST\n");
			memset(buf, 0, sizeof(buf));
			for(i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1)
				{
					sprintf(info, "%d#%s#%ld#", s_users[i].id, s_users[i].name, s_users[i].login_time);
					strcat(buf, info);
				}
			}
			if (strlen(buf) > 0)
			{
				buf[strlen(buf) - 1] = 0;
			}

			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_LIST, n, time(NULL), buf);
			printf("[S->C][processMsg]sendBuff:%s\n",sendBuff);

			write(connfd[n],sendBuff,strlen(sendBuff));
		}
		break;
	case CMD_LOGIN:
		{
			printf("[processMsg]CMD_LOGIN\n");
			s_users[n].id = n;
			s_users[n].login_time = atol(data[OFT_TIM]);
			strcpy(s_users[n].name, data[OFT_FRM]);

			sprintf(sendBuff, "%c:%d:%ld:%s\t%s\0", CMD_LOGIN, n, time(NULL), data[OFT_FRM], "join in");
			printf("[S->C][processMsg]sendBuff:%s\n",sendBuff);

			//����login OK��Ϣ��client
			write(connfd[n],sendBuff,strlen(sendBuff));
			
			//�����û��ļ����֪�����û�
			for(i=0;i<MAX_USERS;i++)
			{
				if(i!=n && connfd[i]!=-1)
				{
					printf("[S->C][processMsg]i:%d notify other users\n",i);
					write(connfd[i],sendBuff,strlen(sendBuff));
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
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_LOGOUT, n, time(NULL), "logout ok!");
			printf("[S->C][processMsg]sendBuff:%s\n",sendBuff);
			write(connfd[n],sendBuff,strlen(sendBuff));
			s_users[n].id = -1;
			strcpy(s_users[n].name, "");
			strcpy(s_users[n].ipaddr, "");
			s_users[n].isPrepared = TRUE;
			printf("[processMsg]exit thread\n");
			close(connfd[n]);
			connfd[n] = -1;
			pthread_exit(0);
		}
		break;
	case CMD_PREPARE:
		{
			printf("[processMsg]CMD_PREPARE\n");
			s_users[n].isPrepared = TRUE;
			sprintf(sendBuff, "%c:%d:%ld:%s", CMD_PREPARE, n, time(NULL), "prepared");
			printf("[S->C][processMsg]sendBuff:%s\n",sendBuff);
			
			hasSomeoneNotPrepared = FALSE;
			memset(sendBuff, 0, sizeof(sendBuff));
			for(i=0;i<MAX_USERS;i++)
			{
				if (s_users[i].id != -1)
				{
					countUsers++;
					if (!s_users[i].isPrepared)
					{
						hasSomeoneNotPrepared = TRUE;
						break;
					}
				}
			}
			
			//�û�������2�������û���׼��������������
			printf("[processMsg]countUsers:%d,will fapai\n",countUsers);
			if (countUsers >= 1 && !hasSomeoneNotPrepared)
			{
				printf("[processMsg]->initializePai\n");
				initializePai(s_cards, COUNT_CARDS);
				printf("[processMsg]->xiPai\n");
				xiPai(s_cards,COUNT_CARDS);
				printf("[processMsg]->faPai\n");
				faPai(s_users, MAX_USERS, s_cards, COUNT_CARDS);
				
				//TODO:send to client
				memset(buf, 0, sizeof(buf));
				for(i=0;i<MAX_USERS;i++)
				{
					if (s_users[i].id != -1)
					{
						sprintf(info, "%d#%s#%d#%d#%d#%d#%d#", s_users[i].id, s_users[i].name, s_users[i].gameInfo.cards[0].id, 
							s_users[i].gameInfo.cards[1].id, s_users[i].gameInfo.cards[2].id, s_users[i].gameInfo.cards[3].id, 
							s_users[i].gameInfo.cards[4].id);
						strcat(buf, info);
					}
				}
				if (strlen(buf) > 0)
				{
					buf[strlen(buf) - 1] = 0;
				}
				sprintf(sendBuff, "%c:%d:%ld:%s", CMD_START, n, time(NULL), buf);
				printf("[S->C][processMsg]sendBuff:%s\n",sendBuff);
				
				for (i=0;i<MAX_USERS;i++)
				{
					if (s_users[i].id != -1)
					{
						write(connfd[i],sendBuff,strlen(sendBuff));
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
			//TODO: ׯ����ξ���?ʱ���Ⱥ������?
			
		}
		break;
	case CMD_STAKE:
		{
			printf("[processMsg]CMD_STAKE\n");
			
		}
		break;
	case CMD_PLAY:
		{
			printf("[processMsg]CMD_PLAY\n");
			
		}
		break;
	default:
		return ERR;
	}

    return OK;
}

