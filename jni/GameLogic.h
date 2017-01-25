#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "DouniuStruct.h"

#ifndef _Included_GameLogic_H
#define _Included_GameLogic_H

extern void initializePai(Card cards[], int countCards);
extern void xiPai(Card cards[], int countCards);
extern void faPai(AllGameInfo allGameInfo[], int maxCountUsers, Card cards[], int countCards);

extern int checkoutStake(DbUserInfo* player, AllGameInfo* playerGameInfo, DbUserInfo* banker, AllGameInfo* bankerGameInfo, char* resultStr);

#endif	//_Included_GameLogic_H