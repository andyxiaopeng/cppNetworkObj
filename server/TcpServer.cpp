#include "TcpServer.h"


TcpServer::TcpServer()
{
	_sock = INVALID_SOCKET;
	_recvCount = 0;
	_msgCount = 0;
	_clientCount = 0;
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
	}
	else
	{
	//	addClientToCellServer(new ClientSocket(cSock));
		ClientSocketPtr tm = std::make_shared<ClientSocket>(cSock);
		addClientToCellServer(tm);
	}
	return cSock;
}

void TcpServer::addClientToCellServer(ClientSocketPtr& pClient)
{
	//���ҿͻ��������ٵ�CellServer��Ϣ�������
	auto pMinServer = _cellServers[0];
	for(auto pCellServer : _cellServers)
	{
		if(pMinServer->getClientCount() > pCellServer->getClientCount())
		{
			pMinServer = pCellServer;
		}
	}
	pMinServer->addClient(pClient);
	OnNetJoin(pClient);
}

void TcpServer::Start(int nCellServer)
{
	for (int n = 0; n < nCellServer; n++)
	{
		//std::shared_ptr<CellServer> ser1 = std::make_shared<CellServer>(_sock);
		//auto ser = new CellServer(_sock);
		auto ser = std::make_shared<CellServer>(_sock);
		_cellServers.push_back(ser);
		//ע�������¼����ܶ���
		ser->setEventObj(this);
		//������Ϣ�����߳�
		ser->Start();
	}
}

bool TcpServer::IsRun()
{
	return _sock != INVALID_SOCKET;
}

bool TcpServer::OnRun()
{
	if (IsRun())
	{
		time4msg();
		// ������select
		// ��������socket���� �ֳ�Ϊ ������
		// fd   ===  file describe   == �ļ�������
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);  // ��_sock�ļ������� ���� fdRead ����

		/// nfds ��һ������ֵ����ָfd_set���������е���������Χ������������
		///	�����������������ֵ+1�� ��windows������ν����һ������ֵ����
		timeval t = { 0,10 }; // ����0������û����Ҫ�ȴ�ʱ�� === ��������
		int ret = select(_sock + 1, &fdRead, 0, 0, &t); // select �����ں��ṩ�ļ������򣬿ͻ��������˷���������������һ�����¼�����select���Ǽ���������¼���
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
			return true;
		}
		return true;
	}
	return false;
}

void TcpServer::time4msg()
{
	auto t1 = _tTime.getElapsedSecond();
	if (t1 >= 1.0)
	{
		std::cout << "thread<"<< _cellServers.size() <<">,time<"<< t1 <<">,socket<"<< _sock <<">,clients<"<< (int)_clientCount <<">,recv<"<< (int)(_recvCount/t1) <<">,msg<"<< (int)(_msgCount / t1) <<">\n";
		_recvCount = 0;
		_msgCount = 0;
		_tTime.update();
	}
}

void TcpServer::OnNetJoin(ClientSocketPtr& pClient)
{
	_clientCount++;
}

void TcpServer::OnNetLeave(ClientSocketPtr& pClient)
{
	_clientCount--;
}

void TcpServer::OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header)
{
	_msgCount++;
}

void TcpServer::OnNetRecv(ClientSocketPtr& pClient)
{
	_recvCount++;
	//printf("client<%d> leave\n", pClient->sockfd());
}

void TcpServer::Close()
{
	if (_sock != INVALID_SOCKET)
	{
	#ifdef _WIN32
		// 8.�ر��׽��� closesocket
		closesocket(_sock);
		//���Windows socket����
		WSACleanup();
	#else
		close(_sock);
	#endif
	}
}
