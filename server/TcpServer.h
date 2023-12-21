#ifndef _TcpServer_
#define _TcpServer_

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
#include <vector>

class TcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> _clients;
public:
	TcpServer();
	virtual ~TcpServer();

	// 初始化socket
	SOCKET InitSocket();
	// 绑定端口号
	int Bind(const char* ip, unsigned short port);
	// 监听端口号
	int Listen(int n);

	// 接收客户端连接
	SOCKET Accept();

	// 关闭socket
	void Close();

	// 查询网络IO
	bool OnRun();

	bool IsRun();

	// 接收信息  处理粘包、拆分包
	int RecvData(SOCKET _cSock);

	// 响应网络信息
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header);

	// 发送信息
	int SendData(SOCKET _cSock, DataHeader* header);

	// 群发信息
	void SendData2All(DataHeader* header);

};

#endif


