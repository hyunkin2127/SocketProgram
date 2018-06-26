#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef SERVER_MAIN_H
#define SERVER_MAIN_H

using namespace std;
#pragma comment(lib, "Ws2_32.lib")

#include <winSock2.h>
#include <iostream>

#include <thread>
#include <chrono>

#include <string>
#include <stdexcept>

#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <vector>


#define SERVER_PORT 11235  // server port number
#define PACKET_DATA_LENGTH 10
#define BUF_SIZE 128 // block transfer size  
#define QUEUE_SIZE 10
#define IPAddress "127.0.0.1" // server IP address
#define PORT_NUM 10200
#define BLOG_SIZE 5
#define MAX_MSG_LEN 256
#define SOCKET_LIST_SIZE  5

#define HEADER_LEN 2;

#endif // !SERVER_MAIN_H