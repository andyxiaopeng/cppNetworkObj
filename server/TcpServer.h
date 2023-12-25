#ifndef _TcpServer_
#define _TcpServer_

#ifdef _WIN32
#define  WIN32_LEAN_AND_MEAN // ����꾡����������һЩ�����������
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>		// linux�ǻ���unix�ģ�����������linux����unix��������һ��unix�ı�׼��
#include <arpa/inet.h>	// linux��������صĿ�
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include <iostream>
#include  <iomanip>
#include "dataType.h"
#include <thread>
#include <vector>
#include "CELLTimestamp.hpp"

//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* szMsgBuf()
	{
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}

private:
	SOCKET _sockfd;
	// �ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SZIE * 10];
	// βָ��
	int _lastPos;
};


class TcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	CELLTimestamp _tTime; // ��ʱ
	int _recvCount; // ����

public:
	TcpServer();
	virtual ~TcpServer();

	// ��ʼ��socket
	SOCKET InitSocket();
	// �󶨶˿ں�
	int Bind(const char* ip, unsigned short port);
	// �����˿ں�
	int Listen(int n);

	// ���տͻ�������
	SOCKET Accept();

	// �ر�socket
	void Close();

	// ��ѯ����IO
	bool OnRun();

	bool IsRun();

	// ������Ϣ  ����ճ������ְ�
	int RecvData(ClientSocket* pClient);

	// ��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header);

	// ������Ϣ
	int SendData(SOCKET _cSock, DataHeader* header);

	// Ⱥ����Ϣ
	void SendData2All(DataHeader* header);

};

#endif


