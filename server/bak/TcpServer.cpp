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
	WORD ver = MAKEWORD(2, 2); // winsocket的版本
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	// 1.创建套接字
	if (_sock != INVALID_SOCKET) // 防止初始化错误
	{
		std::cout << "已存在<socket = " << _sock << ">，现将关闭旧连接……\n";
		Close();
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == INVALID_SOCKET)
	{// 创建失败
		std::cout << "创建socket失败\n";
	}
	else
	{
		std::cout << "创建<socket = " << _sock << ">，成功\n";
	}

	return _sock;
}

int TcpServer::Bind(const char* ip,unsigned short port)
{
	// 2.绑定地址
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
		_sin.sin_addr.S_un.S_addr = INADDR_ANY; // INADDR_ANY 表示本机所有ip都可以访问。 也可以更换为 inet_addr("127.0.0.1");
	}
#else
	if (ip)
	{
		_sin.sin_addr.s_addr = inet_addr(ip); 
	}
	else
	{
		_sin.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY 表示本机所有ip都可以访问。 也可以更换为 inet_addr("127.0.0.1");	
	}
#endif
	int ret = bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{ // 绑定失败
		std::cout << "端口<"<< port <<">绑定失败！\n";
		exit(0);
	}
	else
	{
		// std::cout << "debug:: ip<" << _sin.sin_addr.S_un.S_addr << ">绑定成功！\n";
		std::cout << "端口<" << port << ">绑定成功！\n";
	}
	return ret;
}

int TcpServer::Listen(int n)
{
	// 3.listen 监听网络端口
	int ret = listen(_sock, n);
	if (ret == SOCKET_ERROR)// 最大需要多少人等待链接
	{ // 监听失败
		std::cout << "<socket = " << _sock << ">监听失败！\n";
		exit(0);
	}
	else
	{
		std::cout << "<socket = " << _sock << ">监听成功！\n";
	}
	return ret;
}

SOCKET TcpServer::Accept()
{
	// 4.等待客户端连接
	sockaddr_in clientAddr = {};		// 保存客户端地址信息的对象
	int nAddrLen = sizeof(sockaddr_in);	// 保存客户地址信息的类的长度
	SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
	cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
	cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
	if (cSock == INVALID_SOCKET)
	{
		// 连接失败
		std::cout << "<socket:"<< _sock <<">连接失败！\n";
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
	//查找客户数量最少的CellServer消息处理对象
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
		//注册网络事件接受对象
		ser->setEventObj(this);
		//启动消息处理线程
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
		// 伯克利select
		// 创建三个socket集合 又称为 描述符
		// fd   ===  file describe   == 文件描述符
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);  // 将_sock文件描述符 放入 fdRead 集合

		/// nfds 是一个整数值，是指fd_set集合中所有的描述符范围，而不是数量
		///	既是所有描述符最大值+1， 在windows中无所谓，传一个任意值都行
		timeval t = { 0,10 }; // 两个0，代表没有需要等待时间 === 非阻塞。
		int ret = select(_sock + 1, &fdRead, 0, 0, &t); // select 是由内核提供的监听程序，客户端向服务端发送连接请求本质是一个读事件，而select就是监听这类读事件。
		// ！！！ps：若文件描述符fd（也就是socket）没有任何时间发生，那么在调用select后，fd在集合中的值会被置为0；
		// select()返回结果是fdRead\fdWrite\fdExp这三个集合发生事件的总数.
		if (ret < 0)
		{
			std::cout << "select<socket:" << _sock << "> 有错误！";
			Close();
			return false;
		}
		if (FD_ISSET(_sock, &fdRead)) // 判断文件描述符_sock是否在监听集合fdRead中，且返回fd在集合中的值
		{// fd_Read集合有IO连接了，才启用Socket的 accept 来创建 cSocket，这样就避免了该程序一直被accept阻塞。
			FD_CLR(_sock, &fdRead); // 清除   === 将文件描述符_sock在fdRead的值设为0
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
		// 8.关闭套接字 closesocket
		closesocket(_sock);
		//清除Windows socket环境
		WSACleanup();
	#else
		close(_sock);
	#endif
	}
}
