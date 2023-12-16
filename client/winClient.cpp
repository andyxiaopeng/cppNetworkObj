#define  WIN32_LEAN_AND_MEAN // 这个宏尽量避免早期一些依赖库的引用
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

struct DataPackage
{
	int age;
	char name[32];
};
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
		char msgBuf[4096] = {};
		std::cin >> msgBuf;
		if (strcmp("exit",msgBuf) == 0)
		{
			std::cout << "客户端退出！\n";
			break;
		}
		else
		{
			// 3.发送命令
			send(_sock, msgBuf, 4096, 0);
		}

		// 4.接收服务器信息 recv
		char recvBuf[4096] = {};
		int nLen = recv(_sock, recvBuf, 4096, 0);
		DataPackage* dp = (DataPackage*)recvBuf;
		if (nLen > 0)
		{	// 连接持续中
			// std::cout << recvBuf << std::endl;
			std::cout << "dp.Age: " << dp->age << "   dp.Name:" << dp->name << std::endl;
		}
		else
		{	// 连接断开
			std::cout << "连接断开\n";

		}
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