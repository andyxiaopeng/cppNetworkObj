#define  WIN32_LEAN_AND_MEAN // 这个宏尽量避免早期一些依赖库的引用

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "dataType.h"
#include <vector>


std::vector<SOCKET> g_clients; // 全局clientSocket的存储容器


int prosessor(SOCKET _cSock)
{
	// 缓冲区
	char szRecv[1024] = {};

	// 5.接收信息
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	if (nLen <= 0)
	{
		// 连接失败
		std::cout << "客户端连接断开\n";
		return -1;
	}

	// 6.处理消息
	DataHeader* header = (DataHeader*)szRecv;
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		std::cout << "收到命令： " << header->cmd << "   命令长度为：" << header->dataLength << "   userName: " << login->userName << "    passWord: " << login->passWord << "\n";

		// 先忽略密码判断
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		std::cout << "收到命令： " << header->cmd << "   命令长度为：" << header->dataLength << "   userName: " << logout->userName << "\n";

		// 先忽略密码判断
		LogoutResult ret;
		send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
	}
	break;
	default:
		DataHeader header = { 0,CMD_ERROR };
		send(_cSock, (char*)&header, sizeof(DataHeader), 0);
		break;

	}
}

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


	
	while (true)
	{
		// 伯克利select
		// 创建三个socket集合 又称为 描述符
		// fd   ===  file describe   == 文件描述符
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		for (int n = (int)g_clients.size() - 1;n >= 0;--n)
		{
			FD_SET(g_clients[n], &fdRead); // 把所有和client的socket链接都放入fdRead中
		}

		/// nfds 是一个整数值，是指fd_set集合中所有的描述符范围，而不是数量
		///	既是所有描述符最大值+1， 在windows中无所谓，传一个任意值都行
		timeval t = { 0,0 }; // 两个0，代表没有需要等待时间 === 非阻塞。
		int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,&t);
		if (ret < 0)
		{
			std::cout << "select 有错误！";
			break;
		}
		if (FD_ISSET(_sock,&fdRead)) // 
		{
			FD_CLR(_sock, &fdRead); // 清除   === 将fdRead.fd_count设为0

			// 4.等待客户端连接
			sockaddr_in clientAddr = {};		// 保存客户端地址信息的对象
			int nAddrLen = sizeof(sockaddr_in);	// 保存客户地址信息的类的长度
			SOCKET _cSock = INVALID_SOCKET;

			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (_cSock == INVALID_SOCKET)
			{
				// 连接失败
				std::cout << "连接失败！\n";
				exit(0);
			}
			else
			{
				g_clients.push_back(_cSock);
				std::cout << "ip: " << clientAddr.sin_addr.S_un.S_addr << "  端口：" << clientAddr.sin_port << "   连接成功！\n";
			}
			
		}

		for (int n = 0; n < (int)fdRead.fd_count; ++n)
		{
			if (-1 == prosessor(fdRead.fd_array[n])) // prosessor 返回-1 表示该cSocket链接已经断开，所以需要从vector中将其删除
			{
				auto iter = std::find(g_clients.begin(),g_clients.end(),fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}

		
	}

	for (int n = 0;n < (int)g_clients.size();++n)
	{
		// 关闭全部socket
		closesocket(g_clients[n]);
	}

	// 8.关闭套接字 closesocket
	closesocket(_sock);
	WSACleanup();
	getchar();
}



void startWinServerBase()
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