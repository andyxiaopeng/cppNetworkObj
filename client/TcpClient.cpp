#include "TcpClient.h"

TcpClient::TcpClient()
{
	_sock = INVALID_SOCKET;
}

TcpClient::~TcpClient()
{
	Close();
}

int TcpClient::InitSocket()
{
	int ret = 0;
#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2); // winsocket�İ汾
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	// ���׿ͻ���
	// 1.����socket
	if (_sock != INVALID_SOCKET) // ��ֹ��ʼ������
	{
		std::cout << "�Ѵ���<socket = "<< _sock <<">���ֽ��رվ����ӡ���\n" ;
		Close();
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == INVALID_SOCKET)
	{// ����ʧ��
		std::cout << "����socketʧ��\n";
		ret = 0;
	}
	else
	{
		std::cout << "����socket�ɹ�\n";
		ret = 1;
	}
	return ret;
}

int TcpClient::Connect(const char* ip,const unsigned short port)
{
	int ret = 0;
	if (_sock == INVALID_SOCKET) // ��ֹδ��ʼ��socket�ͽ������Ӳ�����
	{
		InitSocket();
	}

	// 2.���ӷ�����connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip); // 10.60.83.46
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif
	ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); // ���ӷ�����
	if (ret == SOCKET_ERROR)
	{// ����ʧ��
		std::cout << "���ӷ�����ʧ��\n";
	}
	else
	{
		std::cout << "���ӷ������ɹ�\n";
	}
	return  ret;
}

void TcpClient::Close()
{
	if (_sock != INVALID_SOCKET) // ��ֹ���close
	{
		// 4.�ر��׽���closesocket
#ifdef _WIN32
		closesocket(_sock);
		//���Windows socket����
		WSACleanup();
#else
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
		std::cout << "���˳�\n";
	}
}


bool TcpClient::OnRun()
{
	bool ret = false;
	if (IsRun())
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);

		timeval t = { 1,0 };
		ret = select(_sock + 1, &fdRead, 0, 0, &t);
		if (ret < 0)
		{
			std::cout << "<socket:"<< _sock <<">select �д���";
			ret = false;
			return ret;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			ret = RecvData();// ��Ϣ����
			if (ret == -1)
			{
				std::cout << "<socket:" << _sock << ">�������Ͽ�\n";
				ret = false;
				return ret;
			}
		}
		ret = true;
	}
	return ret;
}
bool TcpClient::IsRun()
{
	if (_sock != INVALID_SOCKET)
	{
		return true;
	}
	return false;
}

int TcpClient::RecvData()
{
	int ret = 0;
	// ������
	char szRecv[1024] = {};

	// 5.������Ϣ
	int nLen = recv(_sock, szRecv, sizeof(DataHeader), 0);
	if (nLen <= 0)
	{
		// ����ʧ��
		std::cout << "���������ӶϿ�\n";
		ret = -1;
		return ret;
	}

	// 6.������Ϣ
	DataHeader* header = (DataHeader*)szRecv; // �˴�header��ָ���szRecv�����������ָ�룬���Կ���headerָ�򻺳����ĵ�һ���ڴ��ַ��
	recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);

	OnNetMsg(header);

	ret = 0;
	return ret;
}

void TcpClient::OnNetMsg(DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		LoginResult* loginRet = (LoginResult*)header;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   result: " << loginRet->result << "\n";
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		LogoutResult* logoutRet = (LogoutResult*)header;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   result: " << logoutRet->result << "\n";
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin* nUserRet = (NewUserJoin*)header;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   newUserSocketID: " << nUserRet->sock << "\n";
	}
	break;
	}
}

int TcpClient::SendData(DataHeader* header)
{
	if (IsRun() && header)
	{
		return send(_sock, (char*)header, header->dataLength, 0);
	}
	return  SOCKET_ERROR;
}



void cmdInputThread(TcpClient* client)
{
	while (true)
	{
		char msgBuf[4096] = {};
		std::cin >> msgBuf;

		if (strcmp("exit", msgBuf) == 0)
		{
			client->Close();
			std::cout << " cmdInputThread�߳̽������ͻ����˳���\n";
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

			client->SendData(&login);

		}
		else if (strcmp(msgBuf, "logout") == 0)
		{
			Logout logout;
#ifdef _WIN32
			strcpy_s(logout.userName, "Andy");
#else
			strcpy(logout.userName, "Andy");
#endif
			client->SendData(&logout);
		}
		else
		{
			std::cout << "��֧��ָ�� ���������� \n";
		}
	}
}
