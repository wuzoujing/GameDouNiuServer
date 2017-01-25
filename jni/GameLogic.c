#include "GameLogic.h"
#include "GameRule.h"


void initializePai(Card cards[], int countCards)
{
	int i=0;
	for (i=0;i<countCards;i++)
	{
		//printf("[initializePai]before i:%d\n",i);
		cards[i] = updateCardById(i);
		//printf("[initializePai]end i:%d\n",i);
	}
}

void xiPai(Card cards[], int countCards)
{
	Card temp;
	int pos1;
	int pos2;
	int i=0;
	srand(time(NULL));
	for (i=0;i<COUNT_XIPAI;i++)
	{
		pos1 = rand()%countCards;
		pos2 = rand()%countCards;
		//printf("=pos1: id:%2d value:%2d\n",cards[pos1].id,cards[pos1].value);
		//printf(" pos2: id:%2d value:%2d\n",cards[pos2].id,cards[pos2].value);
		temp = cards[pos1];
		cards[pos1] = cards[pos2];
		cards[pos2] = temp;
	}
	printf("[xiPai]result:\n");
	for (i=0;i<countCards;i++)
	{
		printf(">id:%2d",cards[i].id);
		printf(" value:%2d",cards[i].value);
		printf(" cardValue:%2d",cards[i].cardValue);
		printf(" cardType:%d",cards[i].cardType);
		printf(" cardFace:%2s\n",cards[i].cardFace);
	}
}	


void faPai(AllGameInfo allGameInfo[], int maxCountUsers, Card cards[], int countCards)
{
	int i=0;
	int j=0;
	int index=0;
	for(j=0;j<COUNT_CARD_EACH_PLAYER;j++)
	{
		for (i=0;i<maxCountUsers;i++)
		{
			if (allGameInfo[i].deskId != -1 && allGameInfo[i].tempStatus.isPrepared)
			{
				allGameInfo[i].gameInfo.cards[j] = cards[index];
				index++;
			}
		}
	}
	printf("[faPai]result:\n");
	for (i=0;i<maxCountUsers;i++)
	{
		printf("  user %d\n",i);
		if (allGameInfo[i].deskId != -1 && allGameInfo[i].tempStatus.isPrepared)
		{
			for(j=0;j<COUNT_CARD_EACH_PLAYER;j++)
			{
				printf("    %d->  id:%2d value:%2d\n",j, allGameInfo[i].gameInfo.cards[j].id,allGameInfo[i].gameInfo.cards[j].value);
			}
		}
		else
		{
			printf("    not prepared or offline\n");
		}
	}
}

/*void faPai(GameInfo players[], int countPlayers, Card cards[], int countCards)
{
	int i=0;
	int j=0;
	int index=0;
	for(j=0;j<COUNT_CARD_EACH_PLAYER;j++)
	{
		for (i=0;i<countPlayers;i++)
		{
			players[i].cards[j] = cards[index];
			index++;
		}
	}
	printf("[faPai]result:\n");
	for (i=0;i<countPlayers;i++)
	{
		printf("  player %d\n",i);
		for(j=0;j<COUNT_CARD_EACH_PLAYER;j++)
		{
			printf("    %d->  id:%2d value:%2d\n",j, players[i].cards[j].id,players[i].cards[j].value);
		}
	}
}*/

void calculateResult(GameInfo* gameInfo, bool isHasNiu)
{
	gameInfo->maxCardValue = getMaxCardValue(gameInfo->cards,COUNT_CARD_EACH_PLAYER);
	if (isHasNiu == FALSE)
	{
		gameInfo->pokerPattern = POKER_PATTERN_WU_NIU;
		printf("no need to calculate pattern as wuniu\n");
		return;
	}
	if (checkZhaDan(gameInfo->cards, COUNT_CARD_EACH_PLAYER))
	{
		gameInfo->pokerPattern = POKER_PATTERN_ZHA_DAN;
		printf("zhadan\n");
	}
	else if (checkFiveHua(gameInfo->cards, COUNT_CARD_EACH_PLAYER))
	{
		gameInfo->pokerPattern = POKER_PATTERN_FIVE_HUA;
		printf("fivehua\n");
	}
	else if (checkFourHua(gameInfo->cards, COUNT_CARD_EACH_PLAYER))
	{
		gameInfo->pokerPattern = POKER_PATTERN_FOUR_HUA;
		printf("fourhua\n");
	}
	else
	{
		int points = calculatePoints(gameInfo->cards, COUNT_CARD_EACH_PLAYER);
		switch(points)
		{
			case 0:
				printf("wuniu\n");
				gameInfo->pokerPattern = POKER_PATTERN_WU_NIU;
				break;
			case 1:
				printf("niu 1\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_1;
				break;
			case 2:
				printf("niu 2\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_2;
				break;
			case 3:
				printf("niu 3\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_3;
				break;
			case 4:
				printf("niu 4\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_4;
				break;
			case 5:
				printf("niu 5\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_5;
				break;
			case 6:
				printf("niu 6\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_6;
				break;
			case 7:
				printf("niu 7\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_7;
				break;
			case 8:
				printf("niu 8\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_8;
				break;
			case 9:
				printf("niu 9\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_9;
				break;
			case 10:
				printf("niuniu\n");
				gameInfo->pokerPattern = POKER_PATTERN_NIU_NIU;
				break;
			default:
				printf("no impossible case\n");
				break;
		}
	}
}

int getMultiple(enum POKER_PATTERN pattern)
{
	int multiple = 1;
	if (pattern == POKER_PATTERN_ZHA_DAN ||pattern == POKER_PATTERN_FIVE_HUA)
	{
		multiple = 5;
	}
	else if (pattern == POKER_PATTERN_FOUR_HUA)
	{
		multiple = 4;
	}
	else if (pattern == POKER_PATTERN_NIU_NIU)
	{
		multiple = 3;
	}
	else if (pattern <= POKER_PATTERN_NIU_9 && pattern >= POKER_PATTERN_NIU_7)
	{
		multiple = 2;
	}
	return multiple;
}

// checkout stake between player and banker
// 		return value: money that player win (positive) or lost (negative)
int checkoutStake(DbUserInfo* player, AllGameInfo* playerGameInfo, DbUserInfo* banker, AllGameInfo* bankerGameInfo, char* resultStr)
{
	int ret = 0;
	int multiple = 1;
	if (bankerGameInfo->gameInfo.pokerPattern > playerGameInfo->gameInfo.pokerPattern)
	{
		printf("[checkoutStake]banker win\n");
		strcpy(resultStr, "lost");
		multiple = getMultiple(bankerGameInfo->gameInfo.pokerPattern);
		ret = 0 - playerGameInfo->tempStatus.stake * multiple;
		player->money += ret;
		banker->money -= ret;
		printf("[checkoutStake]banker->money:%ld,player->money:%ld\n",banker->money,player->money);
	}
	else if (bankerGameInfo->gameInfo.pokerPattern < playerGameInfo->gameInfo.pokerPattern)
	{
		printf("[checkoutStake]player win\n");
		strcpy(resultStr, "win");
		multiple = getMultiple(playerGameInfo->gameInfo.pokerPattern);
		ret = playerGameInfo->tempStatus.stake * multiple;
		player->money += ret;
		banker->money -= ret;
		printf("[checkoutStake]banker->money:%ld,player->money:%ld\n",banker->money,player->money);
	}
	else
	{
		if (bankerGameInfo->gameInfo.maxCardValue > playerGameInfo->gameInfo.maxCardValue)
		{
			printf("[checkoutStake]banker win as maxCardValue\n");
			strcpy(resultStr, "lost");
			multiple = getMultiple(bankerGameInfo->gameInfo.pokerPattern);
			ret = 0 - playerGameInfo->tempStatus.stake * multiple;
			player->money += ret;
			banker->money -= ret;
			printf("[checkoutStake]banker->money:%ld,player->money:%ld\n",banker->money,player->money);
		}
		else if (bankerGameInfo->gameInfo.maxCardValue < playerGameInfo->gameInfo.maxCardValue)
		{
			printf("[checkoutStake]player win as maxCardValue\n");
			strcpy(resultStr, "win");
			multiple = getMultiple(playerGameInfo->gameInfo.pokerPattern);
			ret = playerGameInfo->tempStatus.stake * multiple;
			player->money += ret;
			banker->money -= ret;
			printf("[checkoutStake]banker->money:%ld,player->money:%ld\n",banker->money,player->money);
		}
		else
		{
			 printf("[checkoutStake]pattern and maxCard is equal\n");
			 strcpy(resultStr, "equal");
		}
	}
	printf("[checkoutStake]end resultStr:%s\n",resultStr);
	return ret;
}
