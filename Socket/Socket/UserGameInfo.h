#include "main.h"

#pragma once
class UserGameInfo
{
public:
	UserGameInfo();
	~UserGameInfo();

	int betCoin(int betCount);
	const char * BuildGameInfoMsgByState(int state, int round, string user_id);


	int curCoin = 50;
	int bettedCoinThisTurn;

};