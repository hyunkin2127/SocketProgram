#include "main.h"

/*
0:id (ping)
> 0:���� (���� 1:������, 2:��������)
1:id (������)
> ���� 1:10:1:2 (�����ڻ�/����/ȸ��)
2:1:10 (���� 1:Ȧ, 2 ¦)
> ���� 2:id:10:2 (����) << ���� ������ �ʾ����� �ٸ�������� ������Ȳ�� ���� ����.
> ������ ȸ���� ������ ������Ȳ�� ����ؾ��Ѵ�.
> 30ȸ������ ����
> 30ȸ�� ����� ��ο��� �� ����� ����
3:id (������)
*/


#pragma once
class ClientInfo
{
public:
	ClientInfo();
	ClientInfo(SOCKET socket, string id);
	~ClientInfo();

	void clearObj();
	int betCoin(int betCount);
	const char * BuildGameInfoMsgByState(int state, int round, string user_id);


	SOCKET socket;
	string user_id;
	SOCKADDR_IN address;
	//UserGameInfo * gInfo;
	
	int curCoin = 50;
	int bettedCoinThisTurn;


	char sendBuff[MAX_MSG_LEN];
	char recvBuff[MAX_MSG_LEN];
};

