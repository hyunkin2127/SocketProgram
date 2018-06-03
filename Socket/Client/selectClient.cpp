#include "main.h"

#define INIT()	init (argv)
#define isvalidsock(s) ( ( s ) >= 0 )
#define CLOSE(s) if ( closesocket( s ) ) error( 1, errno, "close failed" )
#define EXIT(s)	 exit( s )

static void client(SOCKET s, HANDLE* recvThread);
static void init(char **argv);
static void set_address(char *IP_addr, char *Port_addr, struct sockaddr_in *addr, char *protocol);
static void error(int status, int err, char *fmt, ...);
DWORD WINAPI recvProc(LPVOID lParam);  // 생성한 Thread로 돌릴 함수의 전방 선언

char *program_name;
SOCKET mainSock;
struct sockaddr_in peer;
HANDLE recvThread = NULL; // CreateThread 함수가 성공하면 HANDLE 유형의 thread handler가 반환된다. 이를 저장할 변수
char *IP_addr;
char *Port_addr;

int main(int argc, char** argv)
{
	INIT();

	if (argc != 3)  //올바른 형식으로 입력하지 않은 경우
	{
		error(1, 0, "잘못된 입력형식입니다.\n");
	}
	else //주소와 포트 번호로 호출될 경우
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

	client(mainSock, &recvThread); // client함수로 소켓 기술자와 HANDLE 유형의 변수 recvThread의 주소를 인자로 전달
}

void client(SOCKET s, HANDLE* recvThread)
{
	int state;
	char buff[500] = { NULL };
	char message[1024];
	char userID[10];

	*recvThread = CreateThread(NULL, 0, recvProc, (LPVOID)&s, 0, NULL);
	/* * 인자 설명
	1. NULL : 기본 보안 특성을 사용하기 위해 NULL로 지정
	2. 0 : Thread의 stack 크기를 0으로 초기화
	3. recvProc : Thread에서 수행할 함수
	4. (LPVOID)&s : 3의 Thread에서 실행할 함수에 전달할 매개변수
	5. 0 : Flags 사용
	6. NULL : 새롭게 생성된 Thread의 ID를 얻을 DWORD형 포인터이다. 여기서는 NULL로 설정하여 ID를 생성하지 않았다. */

	printf("Client: Enter your name. > ");
	gets(userID);

	while (1)
	{
		printf("Your Message $ > ");
		gets(buff);     // 메시지를 입력 받음

		if (strcmp(buff, "quit") == 0)   // 입력되는 문자열이 quit 일경우 종료를 위해 이를 비교함
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
			state = send(s, message, strlen(message), 0);      // 입력된 메시지를 서버로 송신
			if (state < 0)                                      // 이하 오류처리 부분
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

DWORD WINAPI recvProc(LPVOID lParam) // 생성된 쓰레드 내부에서 수행할 함수 
{
	SOCKET s = *(SOCKET *)lParam;  //전달받은 매개변수를 소켓디스크립터로 형변환
	int recv_cnt;

	char recv_buf[512] = { NULL };

	while (1)
	{
		recv_cnt = recv(s, recv_buf, sizeof(recv_buf), 0);       // 전달받은 소켓에 대해 recv()함수 수행
		if (recv_cnt == 0)                                        // 이하 오류처리 부분
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
