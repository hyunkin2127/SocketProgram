#include "main.h"

#define INIT()	init (argv)
#define EXIT(s)	 exit( s )
#define CLOSE(s)	if ( closesocket( s ) ) error( 1, errno, "close failed" )
#define isvalidsock(s) ( ( s ) >= 0 )

int rcvbufsz = 2000 * 1600;
char *program_name;
static void set_address(char *IP_addr, char *Port_addr, struct sockaddr_in *addr, char *protocol);
void error(int status, int err, char *fmt, ...);
void init(char **argv);
void server(SOCKET s);

int main(int argc, char **argv)
{
	fd_set mainFD_SET, copiedFD_SET;  // fd_set ����ü ���� ����
	int mainSock, state, fd_set_index, client_len;
	int accecpSock, len;
	int qsize = 5; // ������ ����� �� �ִ� �ִ� client�� ������ 5���� ����
	const int on = 1;
	struct sockaddr_in local;
	struct sockaddr_in peer;
	SOCKET tempSock;
	char *IP_addr; // ������ �ּ� ����
	char *Port_addr; // ��Ʈ��ȣ ����
	char buff[1440];

	INIT();

	if (argc == 2)  // ������ ��Ʈ ��ȣ�θ� ȣ��� ���
	{
		IP_addr = NULL;
		Port_addr = argv[1];
	}
	else // ������ �ּҿ� ��Ʈ ��ȣ�� ȣ��� ���
	{
		IP_addr = argv[1];
		Port_addr = argv[2];
	}
	set_address(IP_addr, Port_addr, &local, "tcp");
	mainSock = socket(AF_INET, SOCK_STREAM, 0);

	if (::bind(mainSock, (struct sockaddr *)&local, sizeof(local)))
		error(1, errno, "bind() failed");
	if (listen(mainSock, qsize))
		error(1, errno, "listen() failed");

	FD_ZERO(&mainFD_SET);  // ���� ����� ���� ��� 0���� �����.
	FD_SET(mainSock, &mainFD_SET);  // ������ ����(����)�ϱ� ���ؼ� �ش� �� 1�� �����.

	while (1)
	{
		copiedFD_SET = mainFD_SET; // fd_set ����ü ������ �����ϱ� ���� mainFD_SET�� copiedFD_SET�� �����Ѵ�.
		state = select(0, &copiedFD_SET, NULL, NULL, NULL);
		// select �Լ��� �����ϸ� ��ȭ�� �߻��� socket�� ������ ��ȯ�ȴ�.(���) [-1 : �����߻� , 0 : TimeOut]
		if (state == SOCKET_ERROR)
		{
			printf("Server: select() failed\n");
			WSACleanup();
			EXIT(1);
		}


		for (fd_set_index = 0; fd_set_index < mainFD_SET.fd_count; fd_set_index++)  // ���ݱ��� ������� ���� ������� ������ŭ for�� �ݺ�
		{
			if (FD_ISSET(mainFD_SET.fd_array[fd_set_index], &copiedFD_SET))  // fd_set_index ��° ������ ���� ����ڰ� 1�̸�...
			{
				if (mainFD_SET.fd_array[fd_set_index] == mainSock)
					// fd_set ����ü ���� fd_set_index ��° ������ ���� ����ڰ� �տ��� socket �Լ��� ������� ���� ����ڿ� ������...
				{
					client_len = sizeof(peer);
					accecpSock = accept(mainSock, (SOCKADDR *)&peer, &client_len);

					if (accecpSock == INVALID_SOCKET)
					{
						printf("server: accept call failed\n");
						WSACleanup();
						EXIT(1);
					}

					FD_SET(accecpSock, &mainFD_SET);
					printf("Server: %d socket client handle was accepted\n", accecpSock);
				}

				else  // data_sock_discriptor�� ���� ���� ��
				{
					state = recv(mainFD_SET.fd_array[fd_set_index], buff, sizeof(buff), 0);
					// ���� ���� client�� ������ ����Ǿ��־ �� client�� �����ϱ� ���� mainFD_SET.fd_array[fd_set_index]�� ���
					buff[state] = NULL; // recv()�� �޽����� �������� NULL�� �߰��Ͽ� send()�� �޽����� ���� �˸���(�����޽��� �ߺ�����)
					tempSock = mainFD_SET.fd_array[fd_set_index];
					if (state <= 0) // client�� "quit"�� �Ἥ ������ ��
					{
						CLOSE(mainFD_SET.fd_array[fd_set_index]);
						printf("Server: %d socket client has disconnected\n", mainFD_SET.fd_array[fd_set_index]);
						FD_CLR(mainFD_SET.fd_array[fd_set_index], &mainFD_SET);
					}
					else  // client�� ���� �޽����� �� ���� ���, recv �Լ��� ���� data�� ����Ʈ ���� ��ȯ
					{
						printf("Server: Message arrived from [%d] socket client handle\n", mainFD_SET.fd_array[fd_set_index]);
						for (int i = 1; i < mainFD_SET.fd_count; i++)  // i =1 : accept�Լ� ��� ���� data ��ȯ�� ���� ����ں��� ����
						{
							if (mainFD_SET.fd_array[i] != tempSock)
							{
								state = send(mainFD_SET.fd_array[i], buff, strlen(buff), 0);
								if (state < 0)
								{
									printf("Server: send failed\n");
									WSACleanup();
									EXIT(1);
								}
								else
									printf("Server: recv successful, send executed to whole client.\n");
							}
						}
					}
				}
			}
		}
	}
}
