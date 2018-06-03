#include "main.h"

#define INIT()	init (argv)
#define isvalidsock(s) ( ( s ) >= 0 )
#define CLOSE(s) if ( closesocket( s ) ) error( 1, errno, "close failed" )
#define EXIT(s)	 exit( s )

static void client(SOCKET s, HANDLE* recvThread);
static void init(char **argv);
static void set_address(char *IP_addr, char *Port_addr, struct sockaddr_in *addr, char *protocol);
static void error(int status, int err, char *fmt, ...);
DWORD WINAPI recvProc(LPVOID lParam);  // ������ Thread�� ���� �Լ��� ���� ����

char *program_name;
SOCKET mainSock;
struct sockaddr_in peer;
HANDLE recvThread = NULL; // CreateThread �Լ��� �����ϸ� HANDLE ������ thread handler�� ��ȯ�ȴ�. �̸� ������ ����
char *IP_addr;
char *Port_addr;

int main(int argc, char** argv)
{
	INIT();

	if (argc != 3)  //�ùٸ� �������� �Է����� ���� ���
	{
		error(1, 0, "�߸��� �Է������Դϴ�.\n");
	}
	else //�ּҿ� ��Ʈ ��ȣ�� ȣ��� ���
	{
		IP_addr = argv[1];
		Port_addr = argv[2];
	}

	if (argc == 3)
	{
		set_address(IP_addr, Port_addr, &peer, "tcp");
		mainSock = socket(AF_INET, SOCK_STREAM, 0);
		if (!isvalidsock(mainSock))
			error(1, errno, "socket call failed");
		if (connect(mainSock, (struct sockaddr *)&peer, sizeof(peer)))
			error(1, errno, "connect failed");
	}
	else
	{
		printf("usage: -$ Client [IP_addr] [Port_addr]\n");
		WSACleanup();
		EXIT(1);
	}

	client(mainSock, &recvThread); // client�Լ��� ���� ����ڿ� HANDLE ������ ���� recvThread�� �ּҸ� ���ڷ� ����
}

void client(SOCKET s, HANDLE* recvThread)
{
	int state;
	char buff[500] = { NULL };
	char message[1024];
	char userID[10];

	*recvThread = CreateThread(NULL, 0, recvProc, (LPVOID)&s, 0, NULL);
	/* * ���� ����
	1. NULL : �⺻ ���� Ư���� ����ϱ� ���� NULL�� ����
	2. 0 : Thread�� stack ũ�⸦ 0���� �ʱ�ȭ
	3. recvProc : Thread���� ������ �Լ�
	4. (LPVOID)&s : 3�� Thread���� ������ �Լ��� ������ �Ű�����
	5. 0 : Flags ���
	6. NULL : ���Ӱ� ������ Thread�� ID�� ���� DWORD�� �������̴�. ���⼭�� NULL�� �����Ͽ� ID�� �������� �ʾҴ�. */

	printf("Client: Enter your name. > ");
	gets(userID);

	while (1)
	{
		printf("Your Message $ > ");
		gets(buff);     // �޽����� �Է� ����

		if (strcmp(buff, "quit") == 0)   // �ԷµǴ� ���ڿ��� quit �ϰ�� ���Ḧ ���� �̸� ����
		{
			TerminateThread(recvProc, NULL);
			printf("Client: exit program.\n");
			CLOSE(s);
			WSACleanup();
			exit(-1);
		}
		else
		{
			sprintf(message, "[%s] : %s", userID, buff);
			state = send(s, message, strlen(message), 0);      // �Էµ� �޽����� ������ �۽�
			if (state < 0)                                      // ���� ����ó�� �κ�
			{
				TerminateThread(recvProc, NULL);
				printf("Client: Send failed\n");
				CLOSE(s);
				WSACleanup();
				exit(-1);
			}
		}
	}
}

DWORD WINAPI recvProc(LPVOID lParam) // ������ ������ ���ο��� ������ �Լ� 
{
	SOCKET s = *(SOCKET *)lParam;  //���޹��� �Ű������� ���ϵ�ũ���ͷ� ����ȯ
	int recv_cnt;

	char recv_buf[512] = { NULL };

	while (1)
	{
		recv_cnt = recv(s, recv_buf, sizeof(recv_buf), 0);       // ���޹��� ���Ͽ� ���� recv()�Լ� ����
		if (recv_cnt == 0)                                        // ���� ����ó�� �κ�
		{
			TerminateThread(GetCurrentThread(), NULL);
			printf("Client: Disconnected\n");
			CLOSE(s);
			WSACleanup();
			exit(-1);
		}
		else if (recv_cnt < 0)
		{
			TerminateThread(GetCurrentThread(), NULL);
			printf("Client: Recv failed\n");
			CLOSE(s);
			WSACleanup();
			exit(-1);
		}
		else
		{
			recv_buf[recv_cnt] = NULL;
			printf("\n%s\n", recv_buf);
		}
	}
}
