#ifdef _WIN32
	#define  WIN32_LEAN_AND_MEAN // 这个宏尽量避免早期一些依赖库的引用
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <Windows.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h>		// linux是基于unix的，所以无论是linux还是unix都会引入一个unix的标准库
	#include <arpa/inet.h>	// linux网络编程相关的库
	#include <string.h>
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <iostream>
#include "dataType.h"
#include <thread>

int prosessor(SOCKET _sock)
{
	// 缓冲区
	char szRecv[1024] = {};

	// 5.接收信息
	int nLen = recv(_sock, szRecv, sizeof(DataHeader), 0);
	if (nLen <= 0)
	{
		// 连接失败
		std::cout << "与服务端连接断开\n";
		return -1;
	}

	// 6.处理消息
	DataHeader* header = (DataHeader*)szRecv;
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginRet = (LoginResult*)szRecv;
		std::cout << "收到 <服务端：" << _sock << "> 消息： " << header->cmd << "   消息长度为：" << header->dataLength << "   result: " << loginRet->result << "\n";

	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult* logoutRet = (LogoutResult*)szRecv;
		std::cout << "收到 <服务端：" << _sock << "> 消息： " << header->cmd << "   消息长度为：" << header->dataLength << "   result: " << logoutRet->result << "\n";

	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* nUserRet = (NewUserJoin*)szRecv;
		std::cout << "收到 <服务端：" << _sock << "> 消息： " << header->cmd << "   消息长度为：" << header->dataLength << "   newUserSocketID: " << nUserRet->sock << "\n";

	}
	break;
	}
	return 0;
}
bool g_Run = true;
void cmdInputThread(SOCKET sock)
{
	while (true)
	{
		char msgBuf[4096] = {};
		std::cin >> msgBuf;

		if (strcmp("exit", msgBuf) == 0)
		{
			std::cout << " cmdInputThread线程结束，客户端退出！\n";
			g_Run = false;
			return;
		}
		else if (strcmp(msgBuf, "login") == 0)
		{
			// 3.发送命令
			Login login;
#ifdef _WIN32
			strcpy_s(login.userName, "Andy");
			strcpy_s(login.passWord, "123456");
#else
			strcpy(login.userName, "Andy");
			strcpy(login.passWord, "123456");
#endif

			send(sock, (char*)&login, login.dataLength, 0);

		}
		else if (strcmp(msgBuf, "logout") == 0)
		{
			Logout logout;
#ifdef _WIN32
			strcpy_s(logout.userName, "Andy");
#else
			strcpy(logout.userName, "Andy");
#endif
			send(sock, (char*)&logout, logout.dataLength, 0);
		}
		else
		{
			std::cout << "不支持指令 请重新输入 \n";
		}
	}
}

void startBaseClient()
{

#ifdef _WIN32
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2); // winsocket的版本
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	// 简易客户端
	// 1.创建socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);

	if (_sock == INVALID_SOCKET)
	{// 创建失败
		std::cout << "创建socket失败\n";
	}
	else
	{
		std::cout << "创建socket成功\n";
	}

	// 2.连接服务器connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // 10.60.83.46
#else
	_sin.sin_addr.s_addr = inet_addr("192.168.3.11");
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); // 连接服务器
	if (ret == SOCKET_ERROR)
	{// 连接失败
		std::cout << "连接服务器失败\n";
	}
	else
	{
		std::cout << "连接服务器成功\n";

	}

	// 客户端加入select模型之后，再进行指令数据输入的话，需要加入多线程
	//cmdInputThread(_sock);
	//std::thread t1 = std::thread(cmdInputThread, _sock);
	std::thread t1(cmdInputThread, _sock);
	t1.detach(); // 使得主从线程分离，两个线程哪个先结束都互不影响，否则主线程先结束，从线程未结束就出错了。

	while (g_Run)
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);

		timeval t = { 1,0 };
		int ret = select(_sock + 1, &fdRead, 0, 0, &t);
		if (ret < 0)
		{
			std::cout << "select 有错误！";
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			int ret = prosessor(_sock);// 消息处理
			if (ret == -1)
			{
				std::cout << "服务器断开\n";
				break;
			}
		}
		//std::cout << "空闲！！！" << std::endl;
		//Sleep(1000);
	}

	// 4.关闭套接字closesocket
#ifdef _WIN32
	closesocket(_sock);
	//清除Windows socket环境
	WSACleanup();
#else
	close(_sock);
#endif

	std::cout << "已退出\n";
	getchar();
	return;
}