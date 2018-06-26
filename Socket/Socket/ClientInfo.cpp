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

int ClientInfo::betCoin(int betCount)
{
	if (curCoin < betCount)
	{
		return -1;
	}

	return curCoin - betCount;
}
