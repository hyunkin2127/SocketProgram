#include "main.h"


SOCKET CreateServerSocket(short pnum, int blog); //
void acceptAndProcPacket(SOCKET sock, SOCKET socketList[]);              //Accept Loop
void ProcSendRecv(void *param);
void SendMsgAllUser(char * msg);
void SendMsgAllUserExceptSender(char * msg, SOCKET senderSocket);

void InitProcSocketList(SOCKET socketList[]);
SOCKET SetSocketToList(SOCKET socketList[], SOCKET sock);
bool UnsetSocketFromList(SOCKET sock);

SOCKET ClientSocketList[SOCKET_LIST_SIZE];

fd_set mainFDSet;
fd_set tempFDSet;

int main()
{

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);                  // 윈속 초기화
	SOCKET ServerlistenSocket = CreateServerSocket(PORT_NUM, BLOG_SIZE); // 서버 소켓 생성

	if (ServerlistenSocket == -1)
	{
		perror("create server socket error");

		WSACleanup();
		return 0;
	}

	FD_ZERO(&mainFDSet);
	FD_SET(ServerlistenSocket, &mainFDSet);
	
	acceptAndProcPacket(ServerlistenSocket, ClientSocketList); //accept 이후 쓰레드생성 및 send, recv 처리

	WSACleanup();
	return 0;
}

SOCKET CreateServerSocket(short pnum, int blog)
{
	SOCKET listenSocket;
	listenSocket = socket(PF_INET, SOCK_STREAM, 0); //소켓 생성
	if (listenSocket == -1)
	{
		return -1;
	}

	//
	//int ret ;
	//ULONG isNonBlocking = 1;
	//ret = ioctlsocket(
	//	listenSocket,        //Non-Blocking으로 변경할 소켓
	//	FIONBIO,       //변경할 소켓의 입출력 모드
	//	&isNonBlocking //넘기는 인자, 여기서는 nonblocking설정 값
	//);

	//if (ret == SOCKET_ERROR)
	//{
	//	perror("thread error");
	//	return -1;
	//}

	//  
	SOCKADDR_IN serverAddr = { 0 }; //소켓 주소

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT_NUM);

	int re = 0;
	re = ::bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); //소켓 주소와 네트워크 인터페이스 결합
	if (re == -1)
	{
		return -1;
	}
	re = listen(listenSocket, blog); //백 로그 큐 설정
	if (re == -1)
	{
		return -1;
	}
	return listenSocket;
}

void acceptAndProcPacket(SOCKET listenSock, SOCKET socketList[])
{
	SOCKET procSocket;
	SOCKADDR_IN clientAddr = { 0 };
	int len = sizeof(clientAddr);
	int maxFD = listenSock + 1;
	InitProcSocketList(socketList);


	while (true)
	{
		tempFDSet = mainFDSet;
		if (select(maxFD, &tempFDSet, 0, 0, 0) < 1) {
			perror("select error : ");
			exit(1);
		}


		for (int targetFD = 0; targetFD < maxFD + 1; targetFD++)
		{
			if (FD_ISSET(targetFD, &tempFDSet))
			{
				if (targetFD == listenSock)
				{
					//소켓 셋 순회중에 서버 리스닝소켓일경우에 accecpt 처리 
					procSocket = accept(listenSock, (SOCKADDR *)&clientAddr, &len);
					if (procSocket < 0)
					{
						perror("accept error : ");
						continue;
					}

					if (SetSocketToList(socketList, procSocket) == -1)
					{
						printf("%s:%d connect fail. list is full \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
						continue;
					};

					// accecpt 된 소켓을 set에 할당
					FD_SET(procSocket, &mainFDSet);
					printf("%s:%d connected \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

					if (procSocket > maxFD)
					{
						maxFD = procSocket;
					}
				}
				else
				{
					ProcSendRecv((void *)procSocket);
				}

			}
		}

	}

	closesocket(listenSock); //소켓 닫기
}

void ProcSendRecv(void *arg)
{
	SOCKET procSocket = (SOCKET)arg;
	SOCKADDR_IN clientAddr = { 0 };
	int len = sizeof(clientAddr);

	char msg[MAX_MSG_LEN] = "";
	const char* ret_msg_buf;
	getpeername(procSocket, (SOCKADDR *)&clientAddr, &len); //상대 소켓 주소 알아내기


	std::string ret_msg = "";
	std::string len_1 = "0";
	std::string len_2 = "0";
	int len_by_header = 0;
	int msg_buff_len = sizeof(msg);

	//while (recv(procSocket, msg, msg_buff_len, 0) > 0) //수신
	if (recv(procSocket, msg, msg_buff_len, 0) > 0) //수신
	{
		//패킷 실제 길이 알아오기
		int len_by_real_data_size = 0;
		int len_by_real_data_size_sum = 0;
		for (int i = len_by_real_data_size; i < sizeof(msg); i++)
		{

			if (msg[i] == '\0')
			{
				break;
			}
			len_by_real_data_size++;

			//전송용 문자열 만들기
			if (i > 1 && msg[i] != '\0')
			{

				ret_msg += msg[i];
			}
		}

		//패킷에 세팅한 데이터부분의 길이 구하기

		len_1 = msg[0];
		len_by_real_data_size--;  // 앞 2칸 패킷길이에쓰기 위해서 제거
		if (sizeof(msg) > 2)
		{
			len_2 = msg[1];
			len_by_real_data_size--; // 앞 2칸 패킷길이에쓰기 위해서 제거
		}
		len_1 += len_2;
		len_by_header = stoi(len_1);
		len_by_real_data_size_sum += len_by_real_data_size;

		printf("%s:%d's len_by_header:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), len_by_header);
		printf("%s:%d's len_by_real_data_size:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), len_by_real_data_size);
		printf("%s:%d's data_size_sum:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), len_by_real_data_size_sum);
		/*
		printf("%s:%d's msg:", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		cout << ret_msg << endl;
		*/
		while (1)
		{
			if (len_by_real_data_size_sum >= len_by_header)
			{
				break;
			}

			if (recv(procSocket, msg, msg_buff_len, 0) <= 0)
			{
				break;
			}

			//패킷 실제 길이 알아오기
			int len_by_real_data_size = 0;
			std::string cur_msg = "";
			for (int i = len_by_real_data_size; i < sizeof(msg); i++)
			{

				if (msg[i] == '\0')
				{
					break;
				}


				//전송용 문자열 만들기
				if (i > 1 && msg[i] != '\0')
				{
					len_by_real_data_size++;
					ret_msg += msg[i];
					cur_msg += msg[i];
				}

			}
			len_by_real_data_size_sum += len_by_real_data_size;

			printf("%s:%d's len_by_real_data_size:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), len_by_real_data_size);
			printf("%s:%d's cur_msg: ", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			cout << cur_msg + '\0' << endl;

		}

		//send(procSocket, msg, sizeof(msg), 0); //송신
		printf("%s:%d's ret_msg:", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		cout << ret_msg + '\0' << endl;
		cout << "-------------------------------------" << endl;

		ret_msg_buf = ret_msg.c_str();
		int msg_len = strlen(ret_msg_buf);
		SendMsgAllUserExceptSender((char*)ret_msg_buf, procSocket);

		len_by_header = 0;
		len_1 = "0";
		len_2 = "0";
		ret_msg = "";
		ret_msg_buf = "";
		msg[0] = '\0';

	}
	else {
		UnsetSocketFromList(procSocket);
		FD_CLR(procSocket, &mainFDSet);
		printf("%s:%d close\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	}
	
	//closesocket(procSocket); //소켓 닫기
	//UnsetSocketFromList(procSocket);

}


void SendMsgAllUser(char * msg)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (ClientSocketList[i] != 0)
		{
			int msg_len = strnlen(msg, BUF_SIZE - 1);
			send(ClientSocketList[i], msg, msg_len, 0);
		}
	}
}

void SendMsgAllUserExceptSender(char * msg, SOCKET senderSocket)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (ClientSocketList[i] != 0 && ClientSocketList[i] != senderSocket)
		{
			int msg_len = strnlen(msg, BUF_SIZE - 1);
			send(ClientSocketList[i], msg, msg_len, 0);
		}
	}
}


void InitProcSocketList(SOCKET socketList[])
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		socketList[i] = 0;
	}
}

SOCKET SetSocketToList(SOCKET socketList[], SOCKET sock)
{
	SOCKET idx = 0;

	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (socketList[i] == 0)
		{
			socketList[i] = sock;
			return idx;
		}
	}

	return -1;
}

bool UnsetSocketFromList(SOCKET sock)
{
	for (int i = 0; i < SOCKET_LIST_SIZE; i++)
	{
		if (ClientSocketList[i] == sock)
		{
			ClientSocketList[i] = 0;
			return true;
		}
	}

	return false;
}