#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "DouniuStruct.h"

#ifndef _Included_GameLogic_H
#define _Included_GameLogic_H

int getLogicValue(int id);
Card updateCardById(int id);
void initializePai();
void xiPai();
void faPai();
int GetMaxCardValue(Card cards[], int n);
bool CheckZhaDan(Card cards[], int n);

#endif	//_Included_GameLogic_H