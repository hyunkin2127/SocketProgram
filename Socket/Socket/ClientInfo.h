#include "main.h"

/*
0:id (ping)
> 0:상태 (상태 1:베팅중, 2:베팅종료)
1:id (방입장)
> 응답 1:10:1:2 (보유자산/상태/회차)
2:1:10 (베팅 1:홀, 2 짝)
> 응답 2:id:10:2 (베팅) << 내가 보내지 않았을때 다른사람들의 베팅현황을 볼수 있음.
> 서버는 회차별 유저의 베팅현황을 기록해야한다.
> 30회차까지 진행
> 30회차 종료시 모두에게 총 결과를 전송
3:id (방퇴장)
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

