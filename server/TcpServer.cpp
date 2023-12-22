#include "TcpServer.h"


TcpServer::TcpServer()
{
	_sock = INVALID_SOCKET;
}

TcpServer::~TcpServer()
{
	Close();
}

SOCKET TcpServer::InitSocket()
{
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2); // winsocket�İ汾
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	// 1.�����׽���
		// 1.����socket
	if (_sock != INVALID_SOCKET) // ��ֹ��ʼ������
	{
		std::cout << "�Ѵ���<socket = " << _sock << ">���ֽ��رվ����ӡ���\n";
		Close();
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == INVALID_SOCKET)
	{// ����ʧ��
		std::cout << "����socketʧ��\n";
	}
	else
	{
		std::cout << "����<socket = " << _sock << ">���ɹ�\n";
	}

	return _sock;
}

int TcpServer::Bind(const char* ip,unsigned short port)
{
	// 2.�󶨵�ַ
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	if (ip)
	{
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
	}
	else
	{
		_sin.sin_addr.S_un.S_addr = INADDR_ANY; // INADDR_ANY ��ʾ��������ip�����Է��ʡ� Ҳ���Ը���Ϊ inet_addr("127.0.0.1");
	}
#else
	if (ip)
	{
		_sin.sin_addr.s_addr = inet_addr(ip); 
	}
	else
	{
		_sin.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY ��ʾ��������ip�����Է��ʡ� Ҳ���Ը���Ϊ inet_addr("127.0.0.1");	
	}
#endif
	int ret = bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{ // ��ʧ��
		std::cout << "�˿�<"<< port <<">��ʧ�ܣ�\n";
		exit(0);
	}
	else
	{
		// std::cout << "debug:: ip<" << _sin.sin_addr.S_un.S_addr << ">�󶨳ɹ���\n";
		std::cout << "�˿�<" << port << ">�󶨳ɹ���\n";
	}
	return ret;
}

int TcpServer::Listen(int n)
{
	// 3.listen ��������˿�
	int ret = listen(_sock, n);
	if (ret == SOCKET_ERROR)// �����Ҫ�����˵ȴ�����
	{ // ����ʧ��
		std::cout << "<socket = " << _sock << ">����ʧ�ܣ�\n";
		exit(0);
	}
	else
	{
		std::cout << "<socket = " << _sock << ">�����ɹ���\n";
	}
	return ret;
}

SOCKET TcpServer::Accept()
{
	// 4.�ȴ��ͻ�������
	sockaddr_in clientAddr = {};		// ����ͻ��˵�ַ��Ϣ�Ķ���
	int nAddrLen = sizeof(sockaddr_in);	// ����ͻ���ַ��Ϣ����ĳ���
	SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
	cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
	cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
	if (cSock == INVALID_SOCKET)
	{
		// ����ʧ��
		std::cout << "<socket:"<< _sock <<">����ʧ�ܣ�\n";
		exit(0);
	}
	else
	{
		// �㲥֪ͨ�����ͻ��ˣ����¿ͻ��˼���
		NewUserJoin nUserJoin = {};
		nUserJoin.sock = cSock;
		SendData2All(&nUserJoin);

		_clients.push_back(new ClientSocket(cSock));
#ifdef _WIN32
		std::cout << "ip: " << clientAddr.sin_addr.S_un.S_addr << "  �˿ڣ�" << clientAddr.sin_port << "   ���ӳɹ���\n";
#else
		std::cout << "ip: " << clientAddr.sin_addr.s_addr << "  �˿ڣ�" << clientAddr.sin_port << "   ���ӳɹ���\n";
#endif

	}
	return cSock;
}

bool TcpServer::IsRun()
{
	if (_sock != INVALID_SOCKET)
	{
		return true;
	}
	return false;
}

bool TcpServer::OnRun()
{
	if (IsRun())
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
		for (int n = (int)_clients.size() - 1; n >= 0; --n)
		{
			FD_SET(_clients[n]->sockfd(), &fdRead); // �����к�client��socket���Ӷ�����fdRead��
			if (maxSock < _clients[n]->sockfd())
			{
				maxSock = _clients[n]->sockfd();
			}
		}

		/// nfds ��һ������ֵ����ָfd_set���������е���������Χ������������
		///	�����������������ֵ+1�� ��windows������ν����һ������ֵ����
		timeval t = { 1,0 }; // ����0������û����Ҫ�ȴ�ʱ�� === ��������
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t); // select �����ں��ṩ�ļ������򣬿ͻ��������˷���������������һ�����¼�����select���Ǽ���������¼���
		// ������ps�����ļ�������fd��Ҳ����socket��û���κ�ʱ�䷢������ô�ڵ���select��fd�ڼ����е�ֵ�ᱻ��Ϊ0��
		// select()���ؽ����fdRead\fdWrite\fdExp���������Ϸ����¼�������.
		if (ret < 0)
		{
			std::cout << "select<socket:" << _sock << "> �д���";
			Close();
			return false;
		}
		if (FD_ISSET(_sock, &fdRead)) // �ж��ļ�������_sock�Ƿ��ڼ�������fdRead�У��ҷ���fd�ڼ����е�ֵ
		{// fd_Read������IO�����ˣ�������Socket�� accept ������ cSocket�������ͱ����˸ó���һֱ��accept������
			FD_CLR(_sock, &fdRead); // ���   === ���ļ�������_sock��fdRead��ֵ��Ϊ0

			Accept();
		}

		for (int n = (int)_clients.size() - 1; n >= 0; --n)
		{
			if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
			{
				if (-1 == RecvData(_clients[n])) // prosessor ����-1 ��ʾ��cSocket�����Ѿ��Ͽ���������Ҫ��vector�н���ɾ��
				{
					auto iter = _clients.begin() + n;
					if (iter != _clients.end())
					{
						delete _clients[n];
						_clients.erase(iter); // ����������ָ����Ԫ��
					}
				}
			}
		}
		//std::cout << "���У�����" << std::endl;
		return true;
	}
	return false;
}

// ������
char _szRecv[RECV_BUFF_SZIE] = {};

int TcpServer::RecvData(ClientSocket* pClient)
{
	// 5.������Ϣ
	int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SZIE, 0);
	if (nLen <= 0)
	{
		// ����ʧ��
		std::cout << "<�ͻ���: " << pClient->sockfd() << ">���ӶϿ�\n";
		return -1;
	}

	// ���ջ��������ݷ�����Ϣ������
	memcpy(pClient->szMsgBuf() + pClient->getLastPos(), _szRecv, nLen);
	// ��Ϣ������βָ�����
	int newPos = pClient->getLastPos() + nLen;
	pClient->setLastPos(newPos);

	while (pClient->getLastPos() >= sizeof(DataHeader))  // �����Ϣ�������������Ƿ��Ѿ��ﵽ��Ϣͷ����
	{
		DataHeader* header = (DataHeader*)pClient->szMsgBuf();
		if (pClient->getLastPos() >= header->dataLength)
		{
			// ������Ϣ������ʣ�����ݵĳ���
			int nSize = pClient->getLastPos() - header->dataLength;
			// ������Ϣ
			OnNetMsg(pClient->sockfd(), header);
			// �����������Ϣ����Ϣ�������Ƴ�
			memcpy(pClient->szMsgBuf(), pClient->szMsgBuf() + header->dataLength, nSize);
			// βָ��ǰ��
			pClient->setLastPos(nSize);
		}
		else
		{
			// ʣ�����ݲ���һ����������Ϣ
			break;
		}
	}
	return 0;
}

void TcpServer::OnNetMsg(SOCKET cSock,DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{

		//Login* login = (Login*)header;
		//std::cout << "�յ� <�ͻ��ˣ�" << cSock << "> ��� " << header->cmd << "   �����Ϊ��" << header->dataLength << "   userName: " << login->userName << "    passWord: " << login->passWord << "\n";

		// �Ⱥ��������ж�
		LoginResult ret;
		SendData(cSock, (DataHeader*)&ret);
		//send(cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		//Logout* logout = (Logout*)header;
		//std::cout << "�յ� <�ͻ��ˣ�" << cSock << "> ��� " << header->cmd << "   �����Ϊ��" << header->dataLength << "   userName: " << logout->userName << "\n";

		// �Ⱥ��������ж�
		LogoutResult ret;
		SendData(cSock, (DataHeader*)&ret);
		//send(cSock, (char*)&ret, sizeof(LogoutResult), 0);
	}
	break;
	default:
		std::cout << "δʶ��ָ�error����,���ݳ���"<< header->dataLength <<"\n";
		//DataHeader header;
		//send(cSock, (char*)&header, sizeof(DataHeader), 0);
		break;
	}
}

int TcpServer::SendData(SOCKET _cSock,DataHeader* header)
{
	if (IsRun() && header)
	{
		return send(_cSock, (char*)header, header->dataLength, 0);
	}
	return  SOCKET_ERROR;
}

void TcpServer::SendData2All(DataHeader* header)
{
	// �㲥֪ͨ�����ͻ��ˣ����¿ͻ��˼���
	for (int n = (int)_clients.size() - 1; n >= 0; --n)
	{
		SendData(_clients[n]->sockfd(), header);
	}
}

void TcpServer::Close()
{
#ifdef _WIN32
	for (int n = 0; n < (int)_clients.size(); ++n)
	{
		// �ر�ȫ��socket
		closesocket(_clients[n]->sockfd());
		delete _clients[n]; // new �����Դ�ŵ����ڴ�ռ䣨���ڴ�ռ���ǵ��Ե��ڴ濨�ǳ���Ŀռ䣬�������new�Ļ������Զ��ŵ�ջ�ڴ�ռ䣬����Ƚ�Сһ��ֻ��һ��M�����new��֮�����Ҫ�ǵ�delete�ͷŵ��ⲿ����Դ����
	}
	// 8.�ر��׽��� closesocket
	closesocket(_sock);
	//���Windows socket����
	WSACleanup();
#else
	for (int n = 0; n < (int)_clients.size(); ++n)
	{
		// �ر�ȫ��socket
		close(_clients[n]->sockfd());
		delete _clients[n];
	}
	close(_sock);
#endif
	_clients.clear(); // ����һ��vector���� 
}
