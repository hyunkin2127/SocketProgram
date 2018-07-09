#include "ClientInfo.h"


class ClientInfoList
{
public:
	ClientInfoList();
	~ClientInfoList();

	bool addToList(ClientInfo * info);
	int removeFromList(ClientInfo * info);
	int removeFromList(SOCKET socket);
	int removeFromList(string user_id);
	
	ClientInfo * GetClientInfoObjBySocket(SOCKET socket);

	void BroadcastMsgExceptSender(char * msg, SOCKET senderSocket);
	
	void BroadcastMsg(char * msg);
	void BroadcastMsg(const char * msg);

	void BroadCastGameState(int state, int round);
	
	void ResetBetInfo();
	void CaculateBetCoin(int round);
	void SendGameResultMsgToUser(int round);

	fd_set mainFDSet;
	ClientInfo * list[SOCKET_LIST_SIZE];

	int curIdx = -2;
	int maxFD = 0;
};

