#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define COUNT_CARDS 52
#define COUNT_XIPAI 1000
#define COUNT_CARD_EACH_PLAYER 5
#define MAX_IP_ADDR_LEN 20
#define MAX_NAME_LEN 32
#define COUNT_ROOMS 3
#define MAX_CLIENTS_SUPPORT 18
#define MAX_USERS_EACH_ROOM 6

#ifndef _Included_DouniuStruct_H
#define _Included_DouniuStruct_H

#ifndef bool
#define bool char
#endif

#define TRUE 1 
#define FALSE 0 

typedef struct CARD {
	int id;				// 牌序号，0~51
	int value;			// 牌对应的值，1~10
	int cardType;		// 方块、梅花、红桃、黑桃
	int cardValue;		// 如1,2,...,10,11,12,13
	char cardFace[4];	// 如方块"0","1",...,"10","J","Q","K"
}Card;


enum POKER_PATTERN {
	POKER_PATTERN_WU_NIU = 0,
	POKER_PATTERN_NIU_1,
	POKER_PATTERN_NIU_2,
	POKER_PATTERN_NIU_3,
	POKER_PATTERN_NIU_4,
	POKER_PATTERN_NIU_5,
	POKER_PATTERN_NIU_6,
	POKER_PATTERN_NIU_7,
	POKER_PATTERN_NIU_8,
	POKER_PATTERN_NIU_9,
	POKER_PATTERN_NIU_NIU,
	POKER_PATTERN_FOUR_HUA = 20,
	POKER_PATTERN_FIVE_HUA = 30,
	POKER_PATTERN_ZHA_DAN = 40
};

enum TRYING_BANKER_STATUS { 
	TBS_NONE,
	TBS_SKIP,
	TBS_TRYING
};

typedef struct GAME_INFO {
	Card cards[COUNT_CARD_EACH_PLAYER];
	enum POKER_PATTERN pokerPattern;	// 优先比较牌型
	int maxCardValue;					// 若无牛，则获取最大的牌，如K,Q,J,10,...,A
}GameInfo;

typedef struct TEMP_GAME_STATUS {
	bool isPrepared;
	enum TRYING_BANKER_STATUS bankerStatus;
	int stake;
	bool hasSubmitResult;
}TempGameStatus;

typedef struct DB_USER_INFO {
	char name[MAX_NAME_LEN];
	long money;
	char mark[MAX_NAME_LEN];//个性签名
	int avatarId;//个人头像
	int level;
	//int uniqueid;
}DbUserInfo;

typedef struct ALL_USER_INFO {
	//int deskId;
	long login_time;
	char ipaddr[MAX_IP_ADDR_LEN];
	DbUserInfo dbInfo;
	//int connfd;
}AllUserInfo;

typedef struct ALL_GAME_INFO {
	int deskId;
	GameInfo gameInfo;
	TempGameStatus tempStatus;
}AllGameInfo;

typedef struct ROOM_INFO {
	int roomId;
	int bankerIndex;
	int clientId[MAX_USERS_EACH_ROOM];
	AllGameInfo allGameInfo[MAX_USERS_EACH_ROOM];
	Card cards[COUNT_CARDS];
}RoomInfo;

#endif	//_Included_DouniuStruct_H