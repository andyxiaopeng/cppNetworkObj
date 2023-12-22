#ifndef _TcpClient_
#define _TcpClient_

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



class TcpClient
{
private:
	SOCKET _sock;

	//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 102400
#endif // !RECV_BUFF_SZIE

	//���ջ�����
	char _szRecv[RECV_BUFF_SZIE];
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SZIE * 10];
	//��Ϣ������������β��λ��
	int _lastPos;

public:
	TcpClient();
	virtual ~TcpClient(); // ������

	// ��ʼ��socket
	int InitSocket();
	// ��������
	int Connect(const char* ip,const unsigned short port);
	// �ر�����
	void Close();

	// ��ѯ����IO
	bool OnRun();

	bool IsRun();

	// ������Ϣ  ����ճ������ְ�
	int RecvData();

	// ��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader* header);
	// ������Ϣ
	int SendData(DataHeader* header);

};


//void cmdInputThread(TcpClient* client);

#endif


