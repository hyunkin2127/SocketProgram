#include "ClientInfoList.h"


ClientInfoList::ClientInfoList()
{
	FD_ZERO(&this->mainFDSet);
	
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		list[i] = new ClientInfo();
	}


}


ClientInfoList::~ClientInfoList()
{
}

bool ClientInfoList::addToList(ClientInfo * info)
{
	FD_SET(info->socket, &mainFDSet);

	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket == 0 )
		{
			list[i] = info;
			curIdx = i;

			if (info->socket > maxFD)
			{
				maxFD = info->socket; // 살펴볼 디스크립터의 범위를 넓히기 위함인듯?
			}
			return true;
		}
	}

	return false;
}


int ClientInfoList::removeFromList(ClientInfo * info)
{
	FD_CLR(info->socket, &mainFDSet);

	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (this->list[i]->socket == info->socket)
		{
			list[i]->clearObj();
			curIdx--;
			return curIdx;
		}
	}
	return -1;
}

int ClientInfoList::removeFromList(SOCKET socket)
{
	FD_CLR(socket, &mainFDSet);

	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket == socket)
		{
			list[i]->clearObj();
			curIdx--;
			return curIdx;
		}
	}
	return -1;
}

int ClientInfoList::removeFromList(string user_id)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->user_id.compare(user_id) == 0)
		{
			list[i]->clearObj();
			curIdx--;
			return curIdx;
		}
	}
	return -1;
}

void ClientInfoList::BroadcastMsgExceptSender(char * msg, SOCKET senderSocket)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket > 0 && list[i]->socket != senderSocket)
		{
			int msg_len = strnlen(msg, BUF_SIZE - 1);
			send(list[i]->socket, msg, msg_len, 0);
		}
	}
}

void ClientInfoList::BroadcastMsg(char * msg)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket > 0)
		{
			int msg_len = strnlen(msg, BUF_SIZE - 1);
			send(list[i]->socket, msg, msg_len, 0);
		}
	}
}


ClientInfo * ClientInfoList::GetClientInfoObjBySocket(SOCKET socket)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket  == socket)
		{
			return list[i];
		}
	}
}

void ClientInfoList::BroadcastMsg(const char * msg)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket > 0)
		{
			int msg_len = strnlen(msg, BUF_SIZE - 1);
			send(list[i]->socket, msg, msg_len, 0);
		}
	}
}

void ClientInfoList::BroadCastGameState(int state, int round)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket > 0)
		{
			const char * msg = list[i]->BuildGameInfoMsgByState(state, round, list[i]->user_id);
			int msg_len = strnlen(msg, BUF_SIZE - 1);
			send(list[i]->socket, msg, msg_len, 0);
		}
	}
}




void ClientInfoList::CaculateBetCoin(int evenOdd)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket > 0)
		{
			list[i]->CaculateBetCoin(evenOdd);
		}
	}
}

void ClientInfoList::SendGameResultMsgToUser(int round)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket > 0)
		{
			const char * msg = list[i]->BuildGameResultMsg(round);
			int msg_len = strnlen(msg, BUF_SIZE - 1);
			send(list[i]->socket, msg, msg_len, 0);
			
			cout << list[i]->user_id + " : ";
			cout << msg << endl;
		}
	}
}




void ClientInfoList::ResetBetInfo()
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (list[i]->socket > 0)
		{
			list[i]->ResetBetInfo();
		}
	}
}