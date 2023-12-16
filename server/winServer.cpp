#define  WIN32_LEAN_AND_MEAN // 这个宏尽量避免早期一些依赖库的引用

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
void startWinServer()
{

	WORD ver = MAKEWORD(2, 2); // winsocket的版本
	WSADATA dat;
	WSAStartup(ver, &dat);
	// 1.创建套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// 创建一个socket套接字

	// 2.绑定地址
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // INADDR_ANY 表示本机所有ip都可以访问。 也可以更换为 inet_addr("127.0.0.1");
	_sin.sin_port = htons(4567);
	if (bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{ // 绑定失败
		std::cout << "绑定失败！\n";
		exit(0);
	}
	else
	{
		std::cout << "绑定端口成功！\n";
	}

	// 3.listen 监听网络端口
	if (listen(_sock, 5) == SOCKET_ERROR)// 最大需要多少人等待链接
	{ // 监听失败
		std::cout << "监听失败！\n";
		exit(0);
	}
	else
	{
		std::cout << "监听成功！\n";
	}

	// 4.等待客户端连接
	sockaddr_in clientAddr = {};		// 保存客户端地址信息的对象
	int nAddrLen = sizeof(sockaddr_in);	// 保存客户地址信息的类的长度
	SOCKET _cSock = INVALID_SOCKET;

	const char msgBuf[] = "hello world";

	while (true)
	{
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (_cSock == INVALID_SOCKET)
		{
			// 连接失败
			std::cout << "连接失败！\n";
			exit(0);
		}
		else
		{
			std::cout << "ip: " << clientAddr.sin_addr.S_un.S_addr << "  端口：" << clientAddr.sin_port << "   连接成功！\n";
		}

		// 5.send发送一条数据
		send(_cSock, msgBuf, strlen(msgBuf) + 1, 0); // strlen() 需要+1 主要是因为strlen不会统计str的结束符，所以发送的时候需要长度加一

	}

	// 6.关闭套接字 closesocket
	closesocket(_sock);

	WSACleanup();
}