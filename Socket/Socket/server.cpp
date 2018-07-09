#include "main.h"
#include "GameManager.h"


SOCKET CreateServerSocket(short pnum, int blog); //
void acceptAndProcPacket(SOCKET sock);              //Accept Loop
void ProcSendRecv(void *param);
void ProcSendRecv2(void *param);
void SendMsgAllUser(char * msg);
void SendMsgAllUserExceptSender(char * msg, SOCKET senderSocket);

void InitProcSocketList(SOCKET socketList[]);
SOCKET SetSocketToList(SOCKET socketList[], SOCKET sock);
bool UnsetSocketFromList(SOCKET sock);
void Tokenize(const string& str, vector<string>& tokens, const string& delimiters);

GameManager * gManager;
SOCKET ClientSocketList[SOCKET_LIST_SIZE];
ClientInfoList * cInfoList;


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
	
	
	
	cInfoList = new ClientInfoList();
	ClientInfo * ServerListen = new ClientInfo();

	ServerListen->socket = ServerlistenSocket;
	cInfoList->addToList(ServerListen);
	
	GameManager * gManager = new GameManager();
	gManager->cInfoList = cInfoList;

	_beginthread(GameManager::ProcBetInThread, 0, (void*)gManager);

	acceptAndProcPacket(ServerlistenSocket); //accept ���� ��������� �� send, recv ó��

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

void acceptAndProcPacket(SOCKET listenSock)
{
	SOCKET procSocket;
	SOCKADDR_IN clientAddr = { 0 };
	int len = sizeof(clientAddr);
	int maxFD = listenSock + 1;
	

	const char * fullMsg = "already full\n";
	const char * AccecptSuccessMsg = "connected to server\n";
	string connectNoticeMsgSuffix = " is connnected\n";

	ClientInfoList copiedCInfoList;


	while (true)
	{
		
		copiedCInfoList = *cInfoList; // ���� set�� �����ϱ� ���ؼ� �����ؼ� ���?


		//���캼 ��ũ���� ����� ��� 
		if (select(maxFD, &copiedCInfoList.mainFDSet, 0, 0, 0) < 1) {
			perror("select error : ");
			exit(1);
		}

		//fd �� ���鼭 ä�� �޽��� ó�� 
		for (int targetFD = 0; targetFD < maxFD + 1; targetFD++)
		{
			if (FD_ISSET(targetFD, &copiedCInfoList.mainFDSet))
			{
				if (targetFD == listenSock)
				{
					//���� �� ��ȸ�߿� ���� fd�� ���� �����׼����ϰ�쿡 accept ó�� 
					procSocket = accept(listenSock, (SOCKADDR *)&clientAddr, &len);
					
					if (procSocket < 0)
					{
						perror("accept error : ");
						continue;
					}

					char user_id[MAX_MSG_LEN] = "";
					int user_id_len = sizeof(user_id);
					
					if (recv(procSocket, user_id, user_id_len, 0) < 0)
					{
						perror("accept error : "); 
						continue;
					}
					
					ClientInfo * cInfo = new ClientInfo(procSocket, user_id);
					if (cInfoList->addToList(cInfo) == false)
					{
						printf("%s:%d connect fail. list is full \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
						send(procSocket, fullMsg, sizeof(fullMsg), 0);
						continue;
					}

									
					printf("%s:%d connected \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
					send(procSocket, AccecptSuccessMsg, sizeof(AccecptSuccessMsg), 0);
					

					// ���� �˸�
					string connectNoticeMsg = cInfo->user_id + connectNoticeMsgSuffix;
					cInfoList->BroadcastMsgExceptSender((char*)connectNoticeMsg.c_str(), procSocket);
					maxFD = cInfoList->maxFD;
				}
				else
				{
					ProcSendRecv((void*)targetFD);
				}

			}
		}

	}

	closesocket(listenSock); //���� �ݱ�
}


void ProcSendRecv(void *arg)
{

	SOCKET senderSocket = (SOCKET)arg;
	SOCKADDR_IN clientAddr = { 0 };
	int len = sizeof(clientAddr);

	char msg[MAX_MSG_LEN] = "";
	const char* ret_msg_buf;
	getpeername(senderSocket, (SOCKADDR *)&clientAddr, &len); //��� ���� �ּ� �˾Ƴ���


	std::string ret_msg = "";
	std::string len_1 = "0";
	std::string len_2 = "0";
	int len_by_header = 0;
	int msg_buff_len = sizeof(msg);

	if (recv(senderSocket, msg, msg_buff_len, 0) > 0) //����
	{
		//��Ŷ ���� ���� �˾ƿ���
		int len_by_real_data_size = 0;
		int len_by_real_data_size_sum = 0;
		for (int i = 0; i < sizeof(msg); i++)
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
		printf("%s:%d's msg:", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		cout << ret_msg << endl;
		cout << "-------------------------------------" << endl;

		
		//���� ó��
		if (ret_msg.substr(0, 3).compare("bet") == 0)
		{
			string msg = ret_msg.substr(4);
			vector<string> parsedBetMsg;
			Tokenize(msg, parsedBetMsg, ":");
			ClientInfo * info = cInfoList->GetClientInfoObjBySocket(senderSocket);
			// bet:Ȧ¦(0,1):�׼�

			for (vector<string>::size_type i = 0; i < parsedBetMsg.size(); ++i)
				cout << parsedBetMsg[i] << endl;
			
			
			info->betType = atoi(parsedBetMsg[0].c_str());
			info->bettedCoinThisTurn = atoi(parsedBetMsg[1].c_str());
		}
		
		ret_msg_buf = ret_msg.c_str();
		int msg_len = strlen(ret_msg_buf);

		cInfoList->BroadcastMsgExceptSender((char*)ret_msg_buf, senderSocket);

	}

}


void Tokenize(const string& str, vector<string>& tokens, const string& delimiters )
{
	tokens.clear();
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}
