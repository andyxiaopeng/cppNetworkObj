#define  WIN32_LEAN_AND_MEAN // ����꾡����������һЩ�����������
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "dataType.h"

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


	while (true)
	{
		char msgBuf[4096] = {};
		std::cin >> msgBuf;
		if (strcmp("exit",msgBuf) == 0)
		{
			std::cout << "�ͻ����˳���\n";
			break;
		}else if (strcmp(msgBuf,"login") == 0)
		{
			// 3.��������
			Login login;
			strcpy_s(login.userName, "Andy");
			strcpy_s(login.passWord, "123456");

			send(_sock, (char*)&login, login.dataLength, 0);

			// 4.���շ�������Ϣ recv
			LoginResult loginResult = {};
			recv(_sock, (char*)&loginResult, sizeof(LoginResult), 0);
			std::cout << "loginResult : " << loginResult.result << std::endl;


			std::cout << "��¼��\n";
			
		}else if (strcmp(msgBuf,"logout") == 0)
		{
			Logout logout;
			strcpy_s(logout.userName, "Andy");

			send(_sock, (char*)&logout, logout.dataLength, 0);

			// 4.���շ�������Ϣ recv
			LogoutResult logoutResult = {};
			recv(_sock, (char*)&logoutResult, sizeof(LogoutResult), 0);
			std::cout << "loginResult : " << logoutResult.result << std::endl;
					   
			std::cout << "�˳���\n";
			
		}
		else
		{
			
			std::cout << "��֧��ָ�� ���������� \n";
		}
	}

	// 4.�ر��׽���closesocket
	closesocket(_sock);

	WSACleanup();

	getchar();

}



void startWinClientBase()
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