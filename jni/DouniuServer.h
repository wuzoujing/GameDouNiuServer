#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
//#include <fcntl.h>
//#include <sys/shm.h>
#include <pthread.h>
#include "msg.h"
#include "GameLogic.h"

#define LISTENQ 5
#define MAXLINE 512
#define MAX_USERS 3
#define NAMELEN 20
#define PORT 6666

#ifndef _Included_DouniuServer_H
#define _Included_DouniuServer_H

extern int listenfd,connfd[MAX_USERS];
extern UserInfo s_users[MAX_USERS];
extern Card s_cards[COUNT_CARDS];

//public
extern int initAndAccept();
extern void disconnect();

//private
extern void quit();
extern void receiveCMD(int n);
extern int processMsg(char* buffer, int n);

#endif	//_Included_DouniuServer_H