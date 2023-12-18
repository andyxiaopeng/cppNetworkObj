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
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

int TcpClient::Connect(const char* ip = (const char*)"127.0.0.1",const unsigned short port = NULL)
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

			ret = RecvData(_sock);// ��Ϣ����
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

int TcpClient::RecvData(SOCKET _sock)
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
	DataHeader* header = (DataHeader*)szRecv;
	recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
	OnNetMsg(szRecv,header);

	ret = 0;
	return ret;
}

void TcpClient::OnNetMsg(char* szRecv,DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		LoginResult* loginRet = (LoginResult*)szRecv;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   result: " << loginRet->result << "\n";
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		LogoutResult* logoutRet = (LogoutResult*)szRecv;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   result: " << logoutRet->result << "\n";
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin* nUserRet = (NewUserJoin*)szRecv;
		std::cout << "�յ� <����ˣ�" << _sock << "> ��Ϣ�� " << header->cmd << "   ��Ϣ����Ϊ��" << header->dataLength << "   newUserSocketID: " << nUserRet->sock << "\n";
	}
	break;
	}
}
