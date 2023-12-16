#define  WIN32_LEAN_AND_MEAN // ����꾡����������һЩ�����������

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")


struct DataPackage
{
	int age;
	char name[32];
};

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
		std::cout << "ip: " << clientAddr.sin_addr.S_un.S_addr << "  �˿ڣ�" << clientAddr.sin_port << "   ���ӳɹ���\n";
	}

	char _recvBuf[4096] = {};
	while (true)
	{
		// 5.������Ϣ
		int nLen = recv(_cSock, _recvBuf, 4096, 0);
		if (nLen <= 0)
		{
			// ����ʧ��
			std::cout << "�ͻ������ӶϿ�\n";
			break;
		}
		std::cout << "�յ���� " << _recvBuf << "\n";

		// 6.������Ϣ
		if (0 == strcmp(_recvBuf, "getInfo"))
		{
			DataPackage dp = {80,"�Ź���"};
			send(_cSock, (const char*)&dp, sizeof(DataPackage), 0);
		}
		else
		{
			char msgBuf[] = "???";
			// 7.send����һ������
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0); // strlen() ��Ҫ+1 ��Ҫ����Ϊstrlen����ͳ��str�Ľ����������Է��͵�ʱ����Ҫ���ȼ�һ
		}
	}

	// 6.�ر��׽��� closesocket
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