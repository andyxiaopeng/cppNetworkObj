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
	int RecvData(SOCKET _cSock);

	// ��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header);

	// ������Ϣ
	int SendData(SOCKET _cSock, DataHeader* header);

	// Ⱥ����Ϣ
	void SendData2All(DataHeader* header);

};

#endif


