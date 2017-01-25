#include "GameRule.h"

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

int getMaxCardValue(Card cards[], int countCards)
{
	int i=0;
	int max = 0;
	for (i=0;i<countCards;i++)
	{
		if(max < cards[i].cardValue)
		{
			max = cards[i].cardValue;
		}
	}
	return max;
}

bool checkZhaDan(Card cards[], int countCards)
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

bool checkFiveHua(Card cards[], int countCards)
{
	int i=0;
	int value;
	for(i=0;i<countCards;i++)
	{
		value = cards[i].cardValue;
		if (value != 11 && value != 12 && value != 13)
		{
			return FALSE;
		}
	}
	printf("[checkFiveHua]is fivehua\n");
	return TRUE;
}

bool checkFourHua(Card cards[], int countCards)
{
	int i=0;
	int value;
	int countMoreThenTen = 0;
	for(i=0;i<countCards;i++)
	{
		value = cards[i].cardValue;
		if (value < 10)
		{
			return FALSE;
		}
		else if (value > 10)
		{
			countMoreThenTen++;
		}
	}
	if (countMoreThenTen >= 4)
	{
		printf("[checkFourHua]is fourhua\n");
		return TRUE;
	}
	return FALSE;
}

int getSumOfCards(Card cards[], int countCards)
{
	int i=0;
	int sum = 0;
	for(i=0;i<countCards;i++)
	{
		sum+=cards[i].value;
	}
	return sum;
}

bool checkHasNiu(Card cards[], int countCards)
{
	int i=0;
	int j=0;
	int sum = 0;
	int value1;
	int value2;
	int temp;
	sum = getSumOfCards(cards,countCards);
	for (i=0;i<countCards-1;i++)//i:0~3
	{
		for (j=i+1;j<countCards;j++)//j:i~4
		{
			value1 = cards[i].value;
			value2 = cards[j].value;
			temp = sum - value1 - value2;
			printf("[checkHasNiu]value1:%d,value2:%d,temp:%d\n",value1,value2,temp);
			if (temp%10==0)
			{
				printf("[checkFourHua]Has Niu\n");
				return TRUE;
			}
		}
	}
	return FALSE;
}

bool checkNiuNiu(Card cards[], int countCards)
{
	bool ret;
	int sum = 0;
	ret = checkHasNiu(cards,countCards);
	if (ret == TRUE)
	{
		sum = getSumOfCards(cards,countCards);
		if (sum%10==0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

bool calculatePoints(Card cards[], int countCards)
{
	int points = 0;
	bool ret = checkHasNiu(cards,countCards);
	if (ret == TRUE)
	{
		points = getSumOfCards(cards,countCards) % 10;
		if (points == 0)
		{
			return 10;
		} else {
			return points;
		}
	}
	return 0;
}
