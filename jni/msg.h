#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#ifndef _Included_Msg_H
#define _Included_Msg_H

#define 	OK			1 
#define 	ERR			0

// msg type
#define   TYPE_SERVER		1000
#define   TYPE_SERVER_STR	"1000"

// msg target string
#define   SERVER_STR		"SERVER"
#define   TO_ALL_STR		"ALL"

//CMD
// cmd between Client and Server
#define 	CMD_LIST			'L'
#define 	CMD_LOGIN			'I'
#define 	CMD_LOGOUT			'O'
#define 	CMD_TOALL 			'A'
#define 	CMD_PREPARE 		'P'
#define 	CMD_TRYINGBANKER 	'B'
#define 	CMD_STAKE 			'S'
#define 	CMD_PLAY 			'Y'

// cmd from Server to Client
#define 	CMD_S2C_USER_IN		'N'
#define 	CMD_S2C_USER_OUT		'U'
#define 	CMD_S2C_USER_PREP		'R'
#define 	CMD_S2C_STAKE_VALUE	'V'
#define 	CMD_S2C_WILL_BANKER	'E'
#define 	CMD_S2C_WILL_STAKE	'K'
#define 	CMD_S2C_WILL_START	'T'
#define 	CMD_S2C_CARD_PATTERN	'F'
#define 	CMD_S2C_GAME_RESULT	'G'

// C, F send to others
#define 	CMD_CHAT			'C'


// CMD:FROM:TIME:DATA 
#define		DATA_LEN	4
#define		OFT_CMD		0
#define		OFT_FRM		1
#define		OFT_TIM		2
#define		OFT_DAT		3
#define		DATA_TOK	":"

// USR_ID:USR_NAME:TIME
#define		USER_DATA_LEN	3
#define		OFT_USR_ID		0
#define		OFT_USR_NM		1
#define		OFT_LOGIN_TM	2
#define		USER_DATA_TOK	"#"

// USR_ID:USR_NAME:CARD1:CARD2:CARD3:CARD4:CARD5
#define		GAME_DATA_LEN	7
//#define		OFT_USR_ID		0
//#define		OFT_USR_NM		1
#define		OFT_CARD1		2
#define		OFT_CARD2		3
#define		OFT_CARD3		4
#define		OFT_CARD4		5
#define		OFT_CARD5		6
#define		GAME_DATA_TOK	"#"

/*typedef struct USER_INFO {
	int id;
	char name[32];
	long login_time;
	bool isPrepared;
	GameInfo gameInfo;
}UserInfo;*/

/*inline char *time2str(long time, char* buf){
	struct tm *t = localtime(&time);
	strftime(buf, 32, "%Y-%m-%d-%H:%M:%S", t);
	return buf;
}*/

// online status
enum status{ online, offline, invisible };

#endif	//_Included_Msg_H