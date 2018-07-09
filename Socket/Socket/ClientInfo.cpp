#include "ClientInfo.h"



ClientInfo::ClientInfo()
{
	this->socket = 0;
}

ClientInfo::ClientInfo(SOCKET socket, string id)
{
	this->socket = socket;
	this->user_id = id;
	this->curCoin = 50;
	//this->gInfo = new UserGameInfo();

	//this->address = addr;
}


ClientInfo::~ClientInfo()
{
	closesocket(this->socket);
}

void ClientInfo::clearObj()
{
	this->socket = 0;
	this->user_id = "";
}

const char * ClientInfo::BuildGameInfoMsgByState(int state, int round, string user_id)
{
	string msg = std::to_string(state);
	msg += "_";
	msg += std::to_string(round);
	msg += "_";
	msg += std::to_string(this->curCoin);
	msg += "_";
	msg += user_id;
	msg += "_";

	return msg.c_str();
}


const char * ClientInfo::BuildGameResultMsg(int round)
{
	static string msg = std::to_string(round) + "ȸ�� ��� : ";

	if (this->betType == -1)
	{
		msg += "������ ";
	}
	else
	{
		if (this->betType == 0)
		{
			msg += "¦�� �����Ͽ�";
		}
		else if (this->betType == 1)
		{
			msg += "Ȧ�� �����Ͽ�";
		}

		msg += std::to_string(this->bettedCoinThisTurn) + "������ ";

		if (this->is_win == true)
		{
			msg += "ȹ���Ͽ����ϴ�";
		}
		else if (this->is_win == false)
		{
			msg += "�Ҿ����ϴ�";
		}
	}
	msg += "�������� "+std::to_string(this->curCoin)+ "��";

	cout << msg << endl;
	return msg.c_str();
}


int ClientInfo::CaculateBetCoin(int resultBetType)
{
	if (betType == resultBetType)
	{
		curCoin += bettedCoinThisTurn;
		is_win = true;
	}
	else
	{
		curCoin -= bettedCoinThisTurn;
		is_win = false;
	}

	return curCoin;
}

void ClientInfo::ResetBetInfo()
{
	bettedCoinThisTurn = 0;
	betType = -1;
}



