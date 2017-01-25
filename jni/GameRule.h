#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "DouniuStruct.h"

#ifndef _Included_GameRule_H
#define _Included_GameRule_H

extern int getLogicValue(int id);
extern Card updateCardById(int id);
extern int getMaxCardValue(Card cards[], int countCards);
extern bool checkZhaDan(Card cards[], int countCards);
extern bool checkFiveHua(Card cards[], int countCards);
extern bool checkFourHua(Card cards[], int countCards);
extern int getSumOfCards(Card cards[], int countCards);
extern bool checkHasNiu(Card cards[], int countCards);
extern bool checkNiuNiu(Card cards[], int countCards);

#endif	//_Included_GameRule_H