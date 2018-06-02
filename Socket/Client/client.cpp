#include "main.h"


void ProcSendMessage(void *arg);
void ProcReceiveMessage(void *arg);


int main(void)
{
	WSADATA wsaData;
	struct sockaddr_in peer; //�����ּҸ� ������ ����ü sockaddr ����ü ����
	int s;                   //socket�Լ��� ���� ������ socket descriptor ���� ���� ����
	int rc;                  //�� �Լ����� �������θ� Ȯ���ϱ� ���� ��ȯ���� ���� ���� ����

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

	rc = connect(s, (struct sockaddr *)&peer, sizeof(peer)); //connect ����
	if (rc)                                                  //connect ���н� ó��
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