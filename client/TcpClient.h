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
#include "CELLTimestamp.hpp"
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>



class TcpClient
{
private:
	SOCKET _sock;
	bool _isConnect;

	//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

	//接收缓冲区
	char _szRecv[RECV_BUFF_SZIE];
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE * 10];
	//消息缓冲区的数据尾部位置
	int _lastPos;

public:
	TcpClient();
	virtual ~TcpClient(); // 虚析构

	// 初始化socket
	int InitSocket();
	// 建立连接
	int Connect(const char* ip,const unsigned short port);
	// 关闭连接
	void Close();

	// 查询网络IO
	bool OnRun();

	bool IsRun();

	// 接收信息  处理粘包、拆分包
	int RecvData(SOCKET cSock);

	// 响应网络信息
	virtual void OnNetMsg(DataHeader* header);
	// 发送信息
	int SendData(DataHeader* header, int nLens);

};


//void cmdInputThread(TcpClient* client);

#endif


