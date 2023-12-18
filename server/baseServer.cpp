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
		std::cout << "�յ� <�ͻ��ˣ�" << _cSock << "> ��� " << header->cmd << "   �����Ϊ��" << header->dataLength << "   userName: " << login->userName << "    passWord: " << login->passWord << "\n";

		// �Ⱥ��������ж�
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		std::cout << "�յ� <�ͻ��ˣ�" << _cSock << "> ��� " << header->cmd << "   �����Ϊ��" << header->dataLength << "   userName: " << logout->userName << "\n";

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
	return 0;
}

void startBaseServer()
{
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2); // winsocket�İ汾
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	// 1.�����׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// ����һ��socket�׽���

	// 2.�󶨵�ַ
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // INADDR_ANY ��ʾ��������ip�����Է��ʡ� Ҳ���Ը���Ϊ inet_addr("127.0.0.1");
#else
	_sin.sin_addr.s_addr = INADDR_ANY;
#endif
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

		FD_SET(_sock, &fdRead);  // ��_sock�ļ������� ���� fdRead ����
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		SOCKET maxSock = _sock;
		for (int n = (int)g_clients.size() - 1; n >= 0; --n)
		{
			FD_SET(g_clients[n], &fdRead); // �����к�client��socket���Ӷ�����fdRead��
#ifdef _WIN32
#else
			if (maxSock < g_clients[n])
			{
				maxSock = g_clients[n];
			}
#endif

		}

		/// nfds ��һ������ֵ����ָfd_set���������е���������Χ������������
		///	�����������������ֵ+1�� ��windows������ν����һ������ֵ����
		timeval t = { 1,0 }; // ����0������û����Ҫ�ȴ�ʱ�� === ��������
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t); // select �����ں��ṩ�ļ������򣬿ͻ��������˷���������������һ�����¼�����select���Ǽ���������¼���
		// ������ps�����ļ�������fd��Ҳ����socket��û���κ�ʱ�䷢������ô�ڵ���select��fd�ڼ����е�ֵ�ᱻ��Ϊ0��
		// select()���ؽ����fdRead\fdWrite\fdExp���������Ϸ����¼�������.
		if (ret < 0)
		{
			std::cout << "select �д���";
			break;
		}
		if (FD_ISSET(_sock, &fdRead)) // �ж��ļ�������_sock�Ƿ��ڼ�������fdRead�У��ҷ���fd�ڼ����е�ֵ
		{// fd_Read������IO�����ˣ�������Socket�� accept ������ cSocket�������ͱ����˸ó���һֱ��accept������
			FD_CLR(_sock, &fdRead); // ���   === ���ļ�������_sock��fdRead��ֵ��Ϊ0

			// 4.�ȴ��ͻ�������
			sockaddr_in clientAddr = {};		// ����ͻ��˵�ַ��Ϣ�Ķ���
			int nAddrLen = sizeof(sockaddr_in);	// ����ͻ���ַ��Ϣ����ĳ���
			SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
			_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
			if (_cSock == INVALID_SOCKET)
			{
				// ����ʧ��
				std::cout << "����ʧ�ܣ�\n";
				exit(0);
			}
			else
			{
				// �㲥֪ͨ�����ͻ��ˣ����¿ͻ��˼���
				for (int n = (int)g_clients.size() - 1; n >= 0; --n)
				{
					NewUserJoin nUserJoin = {};
					nUserJoin.sock = _cSock;
					send(g_clients[n], (char*)&nUserJoin, nUserJoin.dataLength, 0);
				}

				g_clients.push_back(_cSock);
#ifdef _WIN32
				std::cout << "ip: " << clientAddr.sin_addr.S_un.S_addr << "  �˿ڣ�" << clientAddr.sin_port << "   ���ӳɹ���\n";
#else
				std::cout << "ip: " << clientAddr.sin_addr.s_addr << "  �˿ڣ�" << clientAddr.sin_port << "   ���ӳɹ���\n";
#endif

			}
		}

		for (int n = (int)g_clients.size() - 1; n >= 0; --n)
		{
			if (FD_ISSET(g_clients[n],&fdRead))
			{
				if (-1 == prosessor(g_clients[n])) // prosessor ����-1 ��ʾ��cSocket�����Ѿ��Ͽ���������Ҫ��vector�н���ɾ��
				{
					auto iter = g_clients.begin();
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
				}
			}

		}

		//std::cout << "���У�����" << std::endl;
	}

#ifdef _WIN32
	for (int n = 0; n < (int)g_clients.size(); ++n)
	{
		// �ر�ȫ��socket
		closesocket(g_clients[n]);
	}
	// 8.�ر��׽��� closesocket
	closesocket(_sock);
	//���Windows socket����
	WSACleanup();
#else
	for (int n = 0; n < (int)g_clients.size(); ++n)
	{
		// �ر�ȫ��socket
		close(g_clients[n]);
	}
	close(_sock);
#endif

	getchar();
	return;
}