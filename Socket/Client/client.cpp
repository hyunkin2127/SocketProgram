#include "main.h"


void ProcSendMessage(void *arg);
void ProcReceiveMessage(void *arg);


int main(void)
{
	WSADATA wsaData;
	struct sockaddr_in peer; //서버주소를 저장할 구조체 sockaddr 구조체 정의
	int s;                   //socket함수를 통해 생성한 socket descriptor 담을 변수 선언
	int rc;                  //각 함수들의 성공여부를 확인하기 위해 반환값을 담을 변수 선언
	string sendMsg;

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
	

	cout << "enter your name: ";
	cin >> sendMsg;

	
	rc = connect(s, (struct sockaddr *)&peer, sizeof(peer)); //connect 설정
	if (rc)                                                  //connect 실패시 처리
	{
		perror("connect call failed");
		exit(1);
	}
	
	rc = send(s, sendMsg.c_str(), sendMsg.size(), 0);

	if (rc <= 0)
	{
		perror("send call failed");
	}

	//_beginthread(ProcReceiveMessage, 0, (void *)s);
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
		cout << "\n enter bet coin count : ";
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


char ** ParseMsg(char * msg)
{
	char* context = NULL;
	char* parsedMsg[4];

	
	for (int i = 0; i < 4; i++)
	{
		parsedMsg[i] = strtok_s(msg, "_", &context);
		cout << parsedMsg[i] << endl;
	}

	return parsedMsg;
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

			int rc;
								
			int msg_len = strnlen(receiveMsg, BUF_SIZE - 1);
			char ** parsedMsg = ParseMsg(receiveMsg);

			receiveMsg[msg_len] = '\0';
			
			cout << receiveMsg << endl;

			//코인 걸기 로직
			if ((int)parsedMsg[0] == 1 && (int)parsedMsg[1] > 0)
			{
				string msg(parsedMsg[2]);
				cout << msg + " round start" << endl;
				
				string oddEven;
				cout << "\n select odd(1), even (2) : ";
				cin >> oddEven;

				string cointCount;
				cout << "\n enter bet coin count : ";
				cin >> cointCount;
				
				//action_id_홀짝_금액
				string sendMsg = std::to_string(PACKET_DATA_LENGTH)+ "bet_" + oddEven + "_" + cointCount +"_";
				
				
				rc = send(procSocket, sendMsg.c_str(), sendMsg.size(), 0);
				 
				if (rc <= 0)
				{
					perror("send call failed");
				}

				sendMsg.clear();
			}
		}
	}
}


void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
{
	// 맨 첫 글자가 구분자인 경우 무시
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// 구분자가 아닌 첫 글자를 찾는다
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos)
	{
		// token을 찾았으니 vector에 추가한다
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// 구분자를 뛰어넘는다.  "not_of"에 주의하라
		lastPos = str.find_first_not_of(delimiters, pos);
		// 다음 구분자가 아닌 글자를 찾는다
		pos = str.find_first_of(delimiters, lastPos);
	}
}