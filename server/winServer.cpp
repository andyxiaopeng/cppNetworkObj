#define  WIN32_LEAN_AND_MEAN // ����꾡����������һЩ�����������

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "dataType.h"
#include <vector>


std::vector<SOCKET> g_clients; // ȫ��clientSocket�Ĵ洢����


int prosessor(SOCKET _cSock)
{
	// ������
	char szRecv[1024] = {};

	// 5.������Ϣ
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	if (nLen <= 0)
	{
		// ����ʧ��
		std::cout << "�ͻ������ӶϿ�\n";
		return -1;
	}

	// 6.������Ϣ
	DataHeader* header = (DataHeader*)szRecv;
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		std::cout << "�յ���� " << header->cmd << "   �����Ϊ��" << header->dataLength << "   userName: " << login->userName << "    passWord: " << login->passWord << "\n";

		// �Ⱥ��������ж�
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		std::cout << "�յ���� " << header->cmd << "   �����Ϊ��" << header->dataLength << "   userName: " << logout->userName << "\n";

		// �Ⱥ��������ж�
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

	WORD ver = MAKEWORD(2, 2); // winsocket�İ汾
	WSADATA dat;
	WSAStartup(ver, &dat);
	// 1.�����׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// ����һ��socket�׽���

	// 2.�󶨵�ַ
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // INADDR_ANY ��ʾ��������ip�����Է��ʡ� Ҳ���Ը���Ϊ inet_addr("127.0.0.1");
	_sin.sin_port = htons(4567);
	if (bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{ // ��ʧ��
		std::cout << "��ʧ�ܣ�\n";
		exit(0);
	}
	else
	{
		std::cout << "�󶨶˿ڳɹ���\n";
	}

	// 3.listen ��������˿�
	if (listen(_sock, 5) == SOCKET_ERROR)// �����Ҫ�����˵ȴ�����
	{ // ����ʧ��
		std::cout << "����ʧ�ܣ�\n";
		exit(0);
	}
	else
	{
		std::cout << "�����ɹ���\n";
	}


	
	while (true)
	{
		// ������select
		// ��������socket���� �ֳ�Ϊ ������
		// fd   ===  file describe   == �ļ�������
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
			FD_SET(g_clients[n], &fdRead); // �����к�client��socket���Ӷ�����fdRead��
		}

		/// nfds ��һ������ֵ����ָfd_set���������е���������Χ������������
		///	�����������������ֵ+1�� ��windows������ν����һ������ֵ����
		timeval t = { 0,0 }; // ����0������û����Ҫ�ȴ�ʱ�� === ��������
		int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,&t);
		if (ret < 0)
		{
			std::cout << "select �д���";
			break;
		}
		if (FD_ISSET(_sock,&fdRead)) // 
		{
			FD_CLR(_sock, &fdRead); // ���   === ��fdRead.fd_count��Ϊ0

			// 4.�ȴ��ͻ�������
			sockaddr_in clientAddr = {};		// ����ͻ��˵�ַ��Ϣ�Ķ���
			int nAddrLen = sizeof(sockaddr_in);	// ����ͻ���ַ��Ϣ����ĳ���
			SOCKET _cSock = INVALID_SOCKET;

			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (_cSock == INVALID_SOCKET)
			{
				// ����ʧ��
				std::cout << "����ʧ�ܣ�\n";
				exit(0);
			}
			else
			{
				g_clients.push_back(_cSock);
				std::cout << "ip: " << clientAddr.sin_addr.S_un.S_addr << "  �˿ڣ�" << clientAddr.sin_port << "   ���ӳɹ���\n";
			}
			
		}

		for (int n = 0; n < (int)fdRead.fd_count; ++n)
		{
			if (-1 == prosessor(fdRead.fd_array[n])) // prosessor ����-1 ��ʾ��cSocket�����Ѿ��Ͽ���������Ҫ��vector�н���ɾ��
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
		// �ر�ȫ��socket
		closesocket(g_clients[n]);
	}

	// 8.�ر��׽��� closesocket
	closesocket(_sock);
	WSACleanup();
	getchar();
}



void startWinServerBase()
{

	WORD ver = MAKEWORD(2, 2); // winsocket�İ汾
	WSADATA dat;
	WSAStartup(ver, &dat);
	// 1.�����׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// ����һ��socket�׽���

	// 2.�󶨵�ַ
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // INADDR_ANY ��ʾ��������ip�����Է��ʡ� Ҳ���Ը���Ϊ inet_addr("127.0.0.1");
	_sin.sin_port = htons(4567);
	if (bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{ // ��ʧ��
		std::cout << "��ʧ�ܣ�\n";
		exit(0);
	}
	else
	{
		std::cout << "�󶨶˿ڳɹ���\n";
	}

	// 3.listen ��������˿�
	if (listen(_sock, 5) == SOCKET_ERROR)// �����Ҫ�����˵ȴ�����
	{ // ����ʧ��
		std::cout << "����ʧ�ܣ�\n";
		exit(0);
	}
	else
	{
		std::cout << "�����ɹ���\n";
	}

	// 4.�ȴ��ͻ�������
	sockaddr_in clientAddr = {};		// ����ͻ��˵�ַ��Ϣ�Ķ���
	int nAddrLen = sizeof(sockaddr_in);	// ����ͻ���ַ��Ϣ����ĳ���
	SOCKET _cSock = INVALID_SOCKET;

	const char msgBuf[] = "hello world";

	while (true)
	{
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (_cSock == INVALID_SOCKET)
		{
			// ����ʧ��
			std::cout << "����ʧ�ܣ�\n";
			exit(0);
		}
		else
		{
			std::cout << "ip: " << clientAddr.sin_addr.S_un.S_addr << "  �˿ڣ�" << clientAddr.sin_port << "   ���ӳɹ���\n";
		}

		// 5.send����һ������
		send(_cSock, msgBuf, strlen(msgBuf) + 1, 0); // strlen() ��Ҫ+1 ��Ҫ����Ϊstrlen����ͳ��str�Ľ����������Է��͵�ʱ����Ҫ���ȼ�һ

	}

	// 6.�ر��׽��� closesocket
	closesocket(_sock);

	WSACleanup();
}