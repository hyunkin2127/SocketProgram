#include "main.h"


void ProcSendMessage(void *arg);
void ProcReceiveMessage(void *arg);
void Tokenize(const string& str, vector<string>& tokens, const string& delimiters);


int main(void)
{
	WSADATA wsaData;
	struct sockaddr_in peer; //�����ּҸ� ������ ����ü sockaddr ����ü ����
	int s;                   //socket�Լ��� ���� ������ socket descriptor ���� ���� ����
	int rc;                  //�� �Լ����� �������θ� Ȯ���ϱ� ���� ��ȯ���� ���� ���� ����
	string sendMsg;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // ���� �ʱ�ȭ
		return 1;

	memset(&peer, 0, sizeof(peer));                // peer ����ü ��ü�� 0���� �ʱ�ȭ (sin_zero�� ����)

	peer.sin_family = AF_INET;                     //�������� ���� ����
	peer.sin_port = htons(10200);                   //��Ʈ��ȣ ����
	peer.sin_addr.s_addr = inet_addr("127.0.0.1"); //������ IP�� localhost ����

	s = socket(PF_INET, SOCK_STREAM, 0);           // ���� ����

	if (s < 0)                                     // ���� ���� ���н� ó��
	{
		perror("socket call failed");
		exit(1);
	}
	

	cout << "enter your name: ";
	cin >> sendMsg;

	
	rc = connect(s, (struct sockaddr *)&peer, sizeof(peer)); //connect ����
	if (rc)                                                  //connect ���н� ó��
	{
		perror("connect call failed");
		exit(1);
	}
	
	rc = send(s, sendMsg.c_str(), sendMsg.size(), 0);

	if (rc <= 0)
	{
		perror("send call failed");
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
	string sendMsg;

	SOCKET procSocket = (SOCKET)arg;
	while (true)
	{
		cout << "\n enter bet info(bet:even(0) or odd(1):coinCount ";
		cin >> sendMsg;

		if (sendMsg.compare("exit") == 0)
		{
			exit(0);
		}
		
		sendMsg = std::to_string(PACKET_DATA_LENGTH) + sendMsg;

		rc = send(procSocket, sendMsg.c_str(), sendMsg.size(), 0);

		if (rc <= 0)
		{
			perror("send call failed");
		}

		sendMsg.clear();
	}
}



void ProcReceiveMessage(void *arg)
{
	

	SOCKET procSocket = (SOCKET)arg;
	while (true)
	{
		char receiveMsg[BUF_SIZE];
		memset(&receiveMsg, 0, sizeof(receiveMsg));

		if (recv(procSocket, (char*)receiveMsg, sizeof(receiveMsg), 0) <= 0)
		{
			perror("receive call failed");
		}
		else
		{
			int msg_len = strnlen(receiveMsg, BUF_SIZE - 1);
			receiveMsg[msg_len] = '\0';

			cout << "\n : ";
			cout << receiveMsg << endl;
		}
	}
}


void Tokenize(const string& str, vector<string>& tokens, const string& delimiters)
{
	// �� ù ���ڰ� �������� ��� ����
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// �����ڰ� �ƴ� ù ���ڸ� ã�´�
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos)
	{
		// token�� ã������ vector�� �߰��Ѵ�
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// �����ڸ� �پ�Ѵ´�.  "not_of"�� �����϶�
		lastPos = str.find_first_not_of(delimiters, pos);
		// ���� �����ڰ� �ƴ� ���ڸ� ã�´�
		pos = str.find_first_of(delimiters, lastPos);
	}
}