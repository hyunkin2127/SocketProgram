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
	WSAStartup(MAKEWORD(2, 2), &wsadata);                  // ���� �ʱ�ȭ
	SOCKET ServerlistenSocket = CreateServerSocket(PORT_NUM, BLOG_SIZE); // ���� ���� ����

	if (ServerlistenSocket == -1)
	{
		perror("create server socket error");

		WSACleanup();
		return 0;
	}

	FD_ZERO(&mainFDSet);
	FD_SET(ServerlistenSocket, &mainFDSet);
	
	acceptAndProcPacket(ServerlistenSocket, ClientSocketList); //accept ���� ��������� �� send, recv ó��

	WSACleanup();
	return 0;
}

SOCKET CreateServerSocket(short pnum, int blog)
{
	SOCKET listenSocket;
	listenSocket = socket(PF_INET, SOCK_STREAM, 0); //���� ����
	if (listenSocket == -1)
	{
		return -1;
	}

	//
	//int ret ;
	//ULONG isNonBlocking = 1;
	//ret = ioctlsocket(
	//	listenSocket,        //Non-Blocking���� ������ ����
	//	FIONBIO,       //������ ������ ����� ���
	//	&isNonBlocking //�ѱ�� ����, ���⼭�� nonblocking���� ��
	//);

	//if (ret == SOCKET_ERROR)
	//{
	//	perror("thread error");
	//	return -1;
	//}

	//  
	SOCKADDR_IN serverAddr = { 0 }; //���� �ּ�

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT_NUM);

	int re = 0;
	re = ::bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); //���� �ּҿ� ��Ʈ��ũ �������̽� ����
	if (re == -1)
	{
		return -1;
	}
	re = listen(listenSocket, blog); //�� �α� ť ����
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
					//���� �� ��ȸ�߿� ���� �����׼����ϰ�쿡 accecpt ó�� 
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

					// accecpt �� ������ set�� �Ҵ�
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

	closesocket(listenSock); //���� �ݱ�
}

void ProcSendRecv(void *arg)
{
	SOCKET procSocket = (SOCKET)arg;
	SOCKADDR_IN clientAddr = { 0 };
	int len = sizeof(clientAddr);

	char msg[MAX_MSG_LEN] = "";
	const char* ret_msg_buf;
	getpeername(procSocket, (SOCKADDR *)&clientAddr, &len); //��� ���� �ּ� �˾Ƴ���


	std::string ret_msg = "";
	std::string len_1 = "0";
	std::string len_2 = "0";
	int len_by_header = 0;
	int msg_buff_len = sizeof(msg);

	//while (recv(procSocket, msg, msg_buff_len, 0) > 0) //����
	if (recv(procSocket, msg, msg_buff_len, 0) > 0) //����
	{
		//��Ŷ ���� ���� �˾ƿ���
		int len_by_real_data_size = 0;
		int len_by_real_data_size_sum = 0;
		for (int i = len_by_real_data_size; i < sizeof(msg); i++)
		{

			if (msg[i] == '\0')
			{
				break;
			}
			len_by_real_data_size++;

			//���ۿ� ���ڿ� �����
			if (i > 1 && msg[i] != '\0')
			{

				ret_msg += msg[i];
			}
		}

		//��Ŷ�� ������ �����ͺκ��� ���� ���ϱ�

		len_1 = msg[0];
		len_by_real_data_size--;  // �� 2ĭ ��Ŷ���̿����� ���ؼ� ����
		if (sizeof(msg) > 2)
		{
			len_2 = msg[1];
			len_by_real_data_size--; // �� 2ĭ ��Ŷ���̿����� ���ؼ� ����
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

			//��Ŷ ���� ���� �˾ƿ���
			int len_by_real_data_size = 0;
			std::string cur_msg = "";
			for (int i = len_by_real_data_size; i < sizeof(msg); i++)
			{

				if (msg[i] == '\0')
				{
					break;
				}


				//���ۿ� ���ڿ� �����
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

		//send(procSocket, msg, sizeof(msg), 0); //�۽�
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
	
	//closesocket(procSocket); //���� �ݱ�
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