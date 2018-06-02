#include "main.h"


void ProcSendMessage(void *arg);
void ProcReceiveMessage(void *arg);


int main(void)
{
	WSADATA wsaData;
	struct sockaddr_in peer; //서버주소를 저장할 구조체 sockaddr 구조체 정의
	int s;                   //socket함수를 통해 생성한 socket descriptor 담을 변수 선언
	int rc;                  //각 함수들의 성공여부를 확인하기 위해 반환값을 담을 변수 선언

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // 윈속 초기화
		return 1;

	memset(&peer, 0, sizeof(peer));                // peer 구조체 전체를 0으로 초기화 (sin_zero를 위해)

	peer.sin_family = AF_INET;                     //프로토콜 유형 설정
	peer.sin_port = htons(10200);                   //포트번호 설정
	peer.sin_addr.s_addr = inet_addr("127.0.0.1"); //목적지 IP로 localhost 설정

	s = socket(PF_INET, SOCK_STREAM, 0);           // 소켓 생성

	if (s < 0)                                     // 소켓 생성 실패시 처리
	{
		perror("socket call failed");
		exit(1);
	}

	rc = connect(s, (struct sockaddr *)&peer, sizeof(peer)); //connect 설정
	if (rc)                                                  //connect 실패시 처리
	{
		perror("connect call failed");
		exit(1);
	}


	_beginthread(ProcReceiveMessage, 0, (void *)s);
	_beginthread(ProcSendMessage, 0, (void *)s);

	Sleep(10000000);

	WSACleanup();
	exit(0);
}

void ProcSendMessage(void *arg)
{
	int rc;
	char sendMsg[BUF_SIZE];
	memset(&sendMsg, 0, sizeof(sendMsg));

	SOCKET procSocket = (SOCKET)arg;
	while (true)
	{
		cout << "enter msg : ";
		cin >> sendMsg;

		rc = send(procSocket, (const char*)sendMsg, sizeof(sendMsg), 0);

		if (rc <= 0)
		{
			perror("send call failed");
		}
	}
}

void ProcReceiveMessage(void *arg)
{
	char receiveMsg[BUF_SIZE];
	memset(&receiveMsg, 0, sizeof(receiveMsg));

	SOCKET procSocket = (SOCKET)arg;
	while (true)
	{
		if (recv(procSocket, (char*)receiveMsg, sizeof(receiveMsg), 0) <= 0)
		{
			perror("receive call failed");
		}
		else
		{
			int msg_len = strnlen(receiveMsg, BUF_SIZE - 1);
			receiveMsg[msg_len] = '\0';

			cout << "recvMsg : ";
			cout << receiveMsg << endl;
		}
	}
}