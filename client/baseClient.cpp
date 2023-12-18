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

int prosessor(SOCKET _sock)
{
	// ������
	char szRecv[1024] = {};

	// 5.������Ϣ
	int nLen = recv(_sock, szRecv, sizeof(DataHeader), 0);
	if (nLen <= 0)
	{
		// ����ʧ��
		std::cout << "���������ӶϿ�\n";
		return -1;
	}

	// 6.������Ϣ
	DataHeader* header = (DataHeader*)szRecv;
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginRet = (LoginResult*)szRecv;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   result: " << loginRet->result << "\n";

	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult* logoutRet = (LogoutResult*)szRecv;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   result: " << logoutRet->result << "\n";

	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* nUserRet = (NewUserJoin*)szRecv;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   newUserSocketID: " << nUserRet->sock << "\n";

	}
	break;
	}
}
bool g_Run = true;
void cmdInputThread(SOCKET sock)
{
	while (true)
	{
		char msgBuf[4096] = {};
		std::cin >> msgBuf;

		if (strcmp("exit", msgBuf) == 0)
		{
			std::cout << " cmdInputThread�߳̽������ͻ����˳���\n";
			g_Run = false;
			return;
		}
		else if (strcmp(msgBuf, "login") == 0)
		{
			// 3.��������
			Login login;
#ifdef _WIN32
			strcpy_s(login.userName, "Andy");
			strcpy_s(login.passWord, "123456");
#else
			strcpy(login.userName, "Andy");
			strcpy(login.passWord, "123456");
#endif

			send(sock, (char*)&login, login.dataLength, 0);

		}
		else if (strcmp(msgBuf, "logout") == 0)
		{
			Logout logout;
#ifdef _WIN32
			strcpy_s(logout.userName, "Andy");
#else
			strcpy(logout.userName, "Andy");
#endif
			send(sock, (char*)&logout, logout.dataLength, 0);
		}
		else
		{
			std::cout << "��֧��ָ�� ���������� \n";
		}
	}
}

void startBaseClient()
{

#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2); // winsocket�İ汾
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

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
	_sin.sin_port = htons(4567);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
	_sin.sin_addr.s_addr = inet_addr("192.168.3.11");
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); // ���ӷ�����
	if (ret == SOCKET_ERROR)
	{// ����ʧ��
		std::cout << "���ӷ�����ʧ��\n";
	}
	else
	{
		std::cout << "���ӷ������ɹ�\n";

	}

	// �ͻ��˼���selectģ��֮���ٽ���ָ����������Ļ�����Ҫ������߳�
	//cmdInputThread(_sock);
	//std::thread t1 = std::thread(cmdInputThread, _sock);
	std::thread t1(cmdInputThread, _sock);
	t1.detach(); // ʹ�������̷߳��룬�����߳��ĸ��Ƚ���������Ӱ�죬�������߳��Ƚ��������߳�δ�����ͳ����ˡ�

	while (g_Run)
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);

		timeval t = { 1,0 };
		int ret = select(_sock + 1, &fdRead, 0, 0, &t);
		if (ret < 0)
		{
			std::cout << "select �д���";
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			int ret = prosessor(_sock);// ��Ϣ����
			if (ret == -1)
			{
				std::cout << "�������Ͽ�\n";
				break;
			}
		}
		//std::cout << "���У�����" << std::endl;
		//Sleep(1000);
	}

	// 4.�ر��׽���closesocket
#ifdef _WIN32
	closesocket(_sock);
	//���Windows socket����
	WSACleanup();
#else
	close(_sock);
#endif


	std::cout << "���˳�\n";
	getchar();
	return;
}