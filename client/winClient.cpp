#define  WIN32_LEAN_AND_MEAN // ����꾡����������һЩ�����������
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
void startWinClient()
{
	WORD ver = MAKEWORD(2, 2); // winsocket�İ汾
	WSADATA dat;
	WSAStartup(ver, &dat);

	// ���׿ͻ���
	// 1.����socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);

	if (_sock == INVALID_SOCKET)
	{// ����ʧ��
		std::cout << "����socketʧ��\n";
	}
	else
	{
		std::cout << "����socket�ɹ�\n";

	}

	// 2.���ӷ�����connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	_sin.sin_port = htons(4567);
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); // ���ӷ�����
	if (ret == SOCKET_ERROR)
	{// ����ʧ��
		std::cout << "���ӷ�����ʧ��\n";
	}
	else
	{
		std::cout << "���ӷ������ɹ�\n";

	}

	// 3.���շ�������Ϣ recv
	char msgBuf[4096] = {};
	int nLen = recv(_sock, msgBuf, 4096, 0);
	if (nLen > 0)
	{	// ���ӳ�����
		std::cout << msgBuf << std::endl;
	}
	else
	{	// ���ӶϿ�
		std::cout << "���ӶϿ�\n";

	}

	// 4.�ر��׽���closesocket
	closesocket(_sock);

	WSACleanup();

	getchar();

}
