#include "ClientInfoList.h"
#pragma once
class GameManager
{
	static const int MAX_USER = 5;
	const int MAX_ROUND = 30;
	static const int STATE_BETTING = 1;
	static const int STATE_WAIT = 2;
	
	const int BET_TIME = 30;
	const int WAIT_TIME = 5;
	

public:
	GameManager();
	~GameManager();

	static void ProcBetInThread(void * arg);
	void ProcBet();


	int curRound = 1;
	int curState;
	int leftTime = -1;
	ClientInfoList * cInfoList;
};

