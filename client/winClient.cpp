#define  WIN32_LEAN_AND_MEAN // 这个宏尽量避免早期一些依赖库的引用
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "dataType.h"

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
}

void startWinClient()
{
	WORD ver = MAKEWORD(2, 2); // winsocket的版本
	WSADATA dat;
	WSAStartup(ver, &dat);

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
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	_sin.sin_port = htons(4567);
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); // 连接服务器
	if (ret == SOCKET_ERROR)
	{// 连接失败
		std::cout << "连接服务器失败\n";
	}
	else
	{
		std::cout << "连接服务器成功\n";

	}


	while (true)
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);

		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdRead, 0, 0, &t);
		if (ret < 0)
		{
			std::cout << "select 有错误！";
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			int ret = prosessor(_sock);
			if (ret == -1)
			{
				std::cout << "服务器断开\n";
				break;
			}
		}

		std::cout << "空闲！！！" << std::endl;
		Sleep(1000);
		Login login;
		strcpy(login.userName, "Andy");
		strcpy(login.passWord, "123456");
		send(_sock, (char*)&login, login.dataLength, 0);

		// char msgBuf[4096] = {};
		// std::cin >> msgBuf;
		//
		//
		// if (strcmp("exit",msgBuf) == 0)
		// {
		// 	std::cout << "客户端退出！\n";
		// 	break;
		// }else if (strcmp(msgBuf,"login") == 0)
		// {
		// 	// 3.发送命令
		// 	Login login;
		// 	strcpy_s(login.userName, "Andy");
		// 	strcpy_s(login.passWord, "123456");
		//
		// 	send(_sock, (char*)&login, login.dataLength, 0);
		//
		// 	// 4.接收服务器信息 recv
		// 	LoginResult loginResult = {};
		// 	recv(_sock, (char*)&loginResult, sizeof(LoginResult), 0);
		// 	std::cout << "loginResult : " << loginResult.result << std::endl;
		//
		//
		// 	std::cout << "登录！\n";
		// 	
		// }else if (strcmp(msgBuf,"logout") == 0)
		// {
		// 	Logout logout;
		// 	strcpy_s(logout.userName, "Andy");
		//
		// 	send(_sock, (char*)&logout, logout.dataLength, 0);
		//
		// 	// 4.接收服务器信息 recv
		// 	LogoutResult logoutResult = {};
		// 	recv(_sock, (char*)&logoutResult, sizeof(LogoutResult), 0);
		// 	std::cout << "loginResult : " << logoutResult.result << std::endl;
		// 			   
		// 	std::cout << "退出！\n";
		// 	
		// }
		// else
		// {
		// 	
		// 	std::cout << "不支持指令 请重新输入 \n";
		// }
	}

	// 4.关闭套接字closesocket
	closesocket(_sock);

	WSACleanup();

	getchar();

}



void startWinClientBase()
{
	WORD ver = MAKEWORD(2, 2); // winsocket的版本
	WSADATA dat;
	WSAStartup(ver, &dat);

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
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	_sin.sin_port = htons(4567);
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); // 连接服务器
	if (ret == SOCKET_ERROR)
	{// 连接失败
		std::cout << "连接服务器失败\n";
	}
	else
	{
		std::cout << "连接服务器成功\n";

	}

	// 3.接收服务器信息 recv
	char msgBuf[4096] = {};
	int nLen = recv(_sock, msgBuf, 4096, 0);
	if (nLen > 0)
	{	// 连接持续中
		std::cout << msgBuf << std::endl;
	}
	else
	{	// 连接断开
		std::cout << "连接断开\n";

	}

	// 4.关闭套接字closesocket
	closesocket(_sock);

	WSACleanup();

	getchar();

}