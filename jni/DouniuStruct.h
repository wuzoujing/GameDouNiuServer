#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define COUNT_CARDS 52
#define COUNT_XIPAI 1000
#define COUNT_CARD_EACH_PLAYER 5
#define MAX_IP_ADDR_LEN 20
#define MAX_NAME_LEN 32

#ifndef _Included_DouniuStruct_H
#define _Included_DouniuStruct_H

#ifndef bool
#define bool char
#endif

#define TRUE 1 
#define FALSE 0 

typedef struct CARD {
	int id;				// ����ţ�0~51
	int value;			// �ƶ�Ӧ��ֵ��1~10
	int cardType;		// ���顢÷�������ҡ�����
	int cardValue;		// ��1,2,...,10,11,12,13
	char cardFace[4];	// �緽��"0","1",...,"10","J","Q","K"
}Card;


enum POKER_PATTERN {
	POKER_PATTERN_WU_NIU,
	POKER_PATTERN_YOU_NIU,
	POKER_PATTERN_FOUR_HUA,
	POKER_PATTERN_FIVE_HUA,
	POKER_PATTERN_ZHA_DAN
};

typedef struct GAME_INFO {
	Card cards[COUNT_CARD_EACH_PLAYER];
	enum POKER_PATTERN pokerPattern;	// ���ȱȽ�����
	int points;							// ����ţ����ȣ���������
	int maxCardValue;					// ����ţ�����ȡ�����ƣ���K,Q,J,10,...,A
}GameInfo;

typedef struct USER_INFO {
	int id;
	char name[MAX_NAME_LEN];
	char ipaddr[MAX_IP_ADDR_LEN];
	long login_time;
	bool isPrepared;
	GameInfo gameInfo;
}UserInfo;

#endif	//_Included_DouniuStruct_H