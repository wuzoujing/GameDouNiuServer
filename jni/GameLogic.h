#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "DouniuStruct.h"

#ifndef _Included_GameLogic_H
#define _Included_GameLogic_H

extern void initializePai();
extern void xiPai();
extern void faPai();

extern int checkoutStake(UserInfo* player, UserInfo* banker, char* resultStr);

#endif	//_Included_GameLogic_H