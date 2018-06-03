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
	fd_set mainFD_SET, copiedFD_SET;  // fd_set 구조체 변수 선언
	int mainSock, state, fd_set_index, client_len;
	int accecpSock, len;
	int qsize = 5; // 서버와 연결될 수 있는 최대 client의 개수가 5개로 설정
	const int on = 1;
	struct sockaddr_in local;
	struct sockaddr_in peer;
	SOCKET tempSock;
	char *IP_addr; // 서버의 주소 저장
	char *Port_addr; // 포트번호 저장
	char buff[1440];

	INIT();

	if (argc == 2)  // 서버가 포트 번호로만 호출될 경우
	{
		IP_addr = NULL;
		Port_addr = argv[1];
	}
	else // 서버가 주소와 포트 번호로 호출될 경우
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

	FD_ZERO(&mainFD_SET);  // 소켓 기술자 값을 모두 0으로 만든다.
	FD_SET(mainSock, &mainFD_SET);  // 서버가 관측(감시)하기 위해서 해당 만 1로 만든다.

	while (1)
	{
		copiedFD_SET = mainFD_SET; // fd_set 구조체 원본을 보존하기 위해 mainFD_SET를 copiedFD_SET에 복사한다.
		state = select(0, &copiedFD_SET, NULL, NULL, NULL);
		// select 함수가 성공하면 변화가 발생한 socket의 개수가 반환된다.(양수) [-1 : 오류발생 , 0 : TimeOut]
		if (state == SOCKET_ERROR)
		{
			printf("Server: select() failed\n");
			WSACleanup();
			EXIT(1);
		}


		for (fd_set_index = 0; fd_set_index < mainFD_SET.fd_count; fd_set_index++)  // 지금까지 만들어진 소켓 기술자의 개수만큼 for문 반복
		{
			if (FD_ISSET(mainFD_SET.fd_array[fd_set_index], &copiedFD_SET))  // fd_set_index 번째 소켓의 소켓 기술자가 1이면...
			{
				if (mainFD_SET.fd_array[fd_set_index] == mainSock)
					// fd_set 구조체 안의 fd_set_index 번째 소켓의 소켓 기술자가 앞에서 socket 함수로 만들어진 소켓 기술자와 같으면...
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

				else  // data_sock_discriptor가 옳은 값을 때
				{
					state = recv(mainFD_SET.fd_array[fd_set_index], buff, sizeof(buff), 0);
					// 여러 명의 client가 서버에 연결되어있어서 각 client를 구분하기 위해 mainFD_SET.fd_array[fd_set_index]를 사용
					buff[state] = NULL; // recv()한 메시지의 마지막에 NULL을 추가하여 send()시 메시지의 끝을 알린다(이전메시지 중복방지)
					tempSock = mainFD_SET.fd_array[fd_set_index];
					if (state <= 0) // client가 "quit"를 써서 나갔을 때
					{
						CLOSE(mainFD_SET.fd_array[fd_set_index]);
						printf("Server: %d socket client has disconnected\n", mainFD_SET.fd_array[fd_set_index]);
						FD_CLR(mainFD_SET.fd_array[fd_set_index], &mainFD_SET);
					}
					else  // client가 보낸 메시지를 잘 받은 경우, recv 함수가 받은 data의 바이트 수를 반환
					{
						printf("Server: Message arrived from [%d] socket client handle\n", mainFD_SET.fd_array[fd_set_index]);
						for (int i = 1; i < mainFD_SET.fd_count; i++)  // i =1 : accept함수 결과 생긴 data 교환용 소켓 기술자부터 시작
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
