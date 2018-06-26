#include "UserGameInfo.h"
#include "GameManager.h"


UserGameInfo::UserGameInfo()
{
	this->curCoin = 50;
}


UserGameInfo::~UserGameInfo()
{
}

const char * UserGameInfo::BuildGameInfoMsgByState(int state, int round, string user_id)
{
	string msg = std::to_string(state);
	msg += ":";
	msg += std::to_string(round);
	msg += ":";
	msg += std::to_string(this->curCoin);
	msg += ":";
	msg += user_id;
	msg += ":";

	return msg.c_str();
}

int UserGameInfo::betCoin(int betCount)
{
	if (curCoin < betCount)
	{
		return -1;
	}
	
	return curCoin - betCount;
}


