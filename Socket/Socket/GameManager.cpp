#include "GameManager.h"



GameManager::GameManager()
{
	
}


GameManager::~GameManager()
{
}



void GameManager::ProcBetInThread(void * arg) 
{
	GameManager * manager = (GameManager*)arg;
	manager->ProcBet();
}

void GameManager::ProcBet()
{
	while (curRound <= MAX_ROUND)
	{
		curState = STATE_WAIT;
		cInfoList->BroadCastGameState(curState, curRound);

		for (int i = 0; i <= WAIT_TIME; i++)
		{

			int leftTime = WAIT_TIME - i;
		
			string leftTimeMsg = "���ð� :  ";
			leftTimeMsg += std::to_string(leftTime);
			leftTimeMsg += "s";
			cout << leftTimeMsg << endl;
			cInfoList->BroadcastMsg(leftTimeMsg.c_str());
			leftTimeMsg.clear();
			Sleep(1000);
		}


		

		string startNoticeMsg = std::to_string(curRound);
		startNoticeMsg += "ȸ�� ������ �����մϴ�. ���ýð� :";;
		startNoticeMsg += std::to_string(BET_TIME);
		startNoticeMsg += "s";

		curState = STATE_BETTING;
		cout << startNoticeMsg << endl;

		cInfoList->BroadCastGameState(curState, curRound);
		cInfoList->BroadcastMsg(startNoticeMsg.c_str());
		
		for (int i = 0; i <= BET_TIME; i++)
		{
		
			int leftTime = BET_TIME - i;
			if (leftTime % 3 == 0)
			{
				string leftTimeMsg = "���� �ð� :  ";
				leftTimeMsg += std::to_string(leftTime);
				leftTimeMsg += "s";
				cout << leftTimeMsg +"\0" << endl;
				cInfoList->BroadcastMsg(leftTimeMsg.c_str());
				leftTimeMsg.clear();
			}
			Sleep(1000);
			
		}

		curState = STATE_WAIT;
		string leftTimeMsg = "��������. ����... ";
		cout <<  "��������. ����... " << endl;
		cInfoList->BroadcastMsg(leftTimeMsg.c_str());
		cInfoList->BroadCastGameState(curState, curRound);
		curRound++;
	}
}