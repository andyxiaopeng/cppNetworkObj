#ifndef _TcpClient_
#define _TcpClient_

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


class TcpClient
{
private:
	SOCKET _sock;
public:
	TcpClient();
	virtual ~TcpClient(); // 虚析构

	// 初始化socket
	int InitSocket();
	// 建立连接
	int Connect(const char* ip,const unsigned short port);
	// 关闭连接
	void Close();

	// 发送信息

	// 查询网络IO
	bool OnRun();

	bool IsRun();

	// 接收信息  处理粘包、拆分包
	int RecvData();

	// 响应网络信息
	void OnNetMsg(DataHeader* header);

	int SendData(DataHeader* header);

};


void cmdInputThread(TcpClient* client);


#endif


