#include "GameLogic.h"

int getLogicValue(int id)
{
	int value = 0;
	if (id >= 0 && id < 13) 
	{//方块0~12
		value = id + 1;
		if (value > 10) {//J,Q,K
			value = 10;
		}
	}
	else if(id >= 13 && id < 26)
	{//梅花13~25
		value = id +1 - 13;
		if (value > 10) {//J,Q,K
			value = 10;
		}
	}
	else if(id >= 26 && id < 39)
	{//红桃26~38
		value = id + 1 - 26;
		if (value > 10) {//J,Q,K
			value = 10;
		}
	}
	else if(id >= 39 && id < 52)
	{//黑桃39~51
		value = id + 1 - 39;
		if (value > 10) {//J,Q,K
			value = 10;
		}
	}
	return value;
}

Card updateCardById(int id)
{
	Card card;
	card.id = id;
	card.value = getLogicValue(id);
	int num1 = id%13+1;
	int num2 = id/13+1;
	card.cardValue = num1;
	card.cardType = num2;
	if (num1 >= 2 && num1 <= 9)
	{
		sprintf(card.cardFace, "%d", num1);
	}
	else if (num1 == 10)
	{
		strcpy(card.cardFace, "10");//face = "10";
	} 
	else if (num1 == 11)
	{
		strcpy(card.cardFace, "J");//face = "J";
	} 
	else if (num1 == 12)
	{
		strcpy(card.cardFace, "Q");//face = "Q";
	} 
	else if (num1 == 13)
	{
		strcpy(card.cardFace, "K");//face = "K";
	} 
	else if (num1 == 1)
	{
		strcpy(card.cardFace, "A");//face = "A";
	}
	printf(">id:%2d",card.id);
	printf(" value:%2d",card.value);
	printf(" cardValue:%2d",card.cardValue);
	printf(" cardType:%d",card.cardType);
	printf(" cardFace:%2s\n",card.cardFace);
	return card;
}

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


void faPai(UserInfo users[], int maxCountUsers, Card cards[], int countCards)
{
	int i=0;
	int j=0;
	int index=0;
	for(j=0;j<COUNT_CARD_EACH_PLAYER;j++)
	{
		for (i=0;i<maxCountUsers;i++)
		{
			if (users[i].id != -1 && users[i].isPrepared)
			{
				users[i].gameInfo.cards[j] = cards[index];	
				index++;
			}
		}
	}
	printf("[faPai]result:\n");
	for (i=0;i<maxCountUsers;i++)
	{
		printf("  user %d\n",i);
		if (users[i].id != -1 && users[i].isPrepared)
		{
			for(j=0;j<COUNT_CARD_EACH_PLAYER;j++)
			{
				printf("    %d->  id:%2d value:%2d\n",j, users[i].gameInfo.cards[j].id,users[i].gameInfo.cards[j].value);
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

int GetMaxCardValue(Card cards[], int countCards)
{
	int max = 0;
	int i=0;
	for (i=0;i<countCards;i++)
	{
		if(max < cards[i].cardValue)
		{
			max = cards[i].cardValue;
		}
	}
	return max;
}

bool CheckZhaDan(Card cards[], int countCards)
{
	int i=0;
	int index;
	int oldValue;
	int countList[13];
	for(i=0;i<13;i++)
	{
		countList[i]=0;
	}
	for(i=0;i<countCards;i++)
	{
		index=cards[i].cardValue-1;
		countList[index]++;
	}
	for(i=0;i<13;i++)
	{
		printf("[CheckZhaDan]i:%d,value:%d\n",i,countList[i]);
		if(countList[i]>=4)
		{
			printf("[CheckZhaDan]is zhadan\n");
			return TRUE;
		}
	}
	return FALSE;
}
