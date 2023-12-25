#include "TcpServer.h"


TcpServer::TcpServer()
{
	_sock = INVALID_SOCKET;
	_recvCount = 0;
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
		// 1.创建socket
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
		exit(0);
	}
	else
	{
		// 广播通知其他客户端，有新客户端加入
		NewUserJoin nUserJoin = {};
		nUserJoin.sock = cSock;
		SendData2All(&nUserJoin);

		_clients.push_back(new ClientSocket(cSock));
#ifdef _WIN32
		std::cout << "ip: " << clientAddr.sin_addr.S_un.S_addr << "  端口：" << clientAddr.sin_port << "   连接成功！\n";
#else
		std::cout << "ip: " << clientAddr.sin_addr.s_addr << "  端口：" << clientAddr.sin_port << "   连接成功！\n";
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
		// 伯克利select
		// 创建三个socket集合 又称为 描述符
		// fd   ===  file describe   == 文件描述符
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);  // 将_sock文件描述符 放入 fdRead 集合
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		SOCKET maxSock = _sock;
		for (int n = (int)_clients.size() - 1; n >= 0; --n)
		{
			FD_SET(_clients[n]->sockfd(), &fdRead); // 把所有和client的socket链接都放入fdRead中
			if (maxSock < _clients[n]->sockfd())
			{
				maxSock = _clients[n]->sockfd();
			}
		}

		/// nfds 是一个整数值，是指fd_set集合中所有的描述符范围，而不是数量
		///	既是所有描述符最大值+1， 在windows中无所谓，传一个任意值都行
		timeval t = { 1,0 }; // 两个0，代表没有需要等待时间 === 非阻塞。
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t); // select 是由内核提供的监听程序，客户端向服务端发送连接请求本质是一个读事件，而select就是监听这类读事件。
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
		}

		for (int n = (int)_clients.size() - 1; n >= 0; --n)
		{
			if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
			{
				if (-1 == RecvData(_clients[n])) // prosessor 返回-1 表示该cSocket链接已经断开，所以需要从vector中将其删除
				{
					auto iter = _clients.begin() + n;
					if (iter != _clients.end())
					{
						delete _clients[n];
						_clients.erase(iter); // 从容器擦除指定的元素
					}
				}
			}
		}
		//std::cout << "空闲！！！" << std::endl;
		return true;
	}
	return false;
}

// 缓冲区
char _szRecv[RECV_BUFF_SZIE] = {};

int TcpServer::RecvData(ClientSocket* pClient)
{
	// 5.接收信息
	int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SZIE, 0);
	if (nLen <= 0)
	{
		// 连接失败
		std::cout << "<客户端: " << pClient->sockfd() << ">连接断开\n";
		return -1;
	}

	// 接收缓冲区数据放入消息缓冲区
	memcpy(pClient->szMsgBuf() + pClient->getLastPos(), _szRecv, nLen);
	// 消息缓冲区尾指针后移
	int newPos = pClient->getLastPos() + nLen;
	pClient->setLastPos(newPos);

	while (pClient->getLastPos() >= sizeof(DataHeader))  // 检查消息缓冲区内数据是否已经达到消息头长度
	{
		DataHeader* header = (DataHeader*)pClient->szMsgBuf();
		if (pClient->getLastPos() >= header->dataLength)
		{
			// 计算消息缓冲区剩余数据的长度
			int nSize = pClient->getLastPos() - header->dataLength;
			// 处理消息
			OnNetMsg(pClient->sockfd(), header);
			// 将处理过的消息从消息缓存区移除
			memcpy(pClient->szMsgBuf(), pClient->szMsgBuf() + header->dataLength, nSize);
			// 尾指针前移
			pClient->setLastPos(nSize);
		}
		else
		{
			// 剩余数据不够一条完整的消息
			break;
		}
	}
	return 0;
}

void TcpServer::OnNetMsg(SOCKET cSock,DataHeader* header)
{
	_recvCount++;
	auto t1 = _tTime.getElapsedSend();
	if (t1 >= 1.0)
	{
		std::cout << "time<"<< t1 << std::setprecision(6) <<">   " << ",socket<"<< _sock <<">   " << ",recvCount<"<< _recvCount <<">   \n";
		_tTime.update();
		_recvCount = 0;
	}


	switch (header->cmd)
	{
	case CMD_LOGIN:
	{

		//Login* login = (Login*)header;
		//std::cout << "收到 <客户端：" << cSock << "> 命令： " << header->cmd << "   命令长度为：" << header->dataLength << "   userName: " << login->userName << "    passWord: " << login->passWord << "\n";

		// 先忽略密码判断
		LoginResult ret;
		SendData(cSock, (DataHeader*)&ret);
		//send(cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		//Logout* logout = (Logout*)header;
		//std::cout << "收到 <客户端：" << cSock << "> 命令： " << header->cmd << "   命令长度为：" << header->dataLength << "   userName: " << logout->userName << "\n";

		// 先忽略密码判断
		LogoutResult ret;
		SendData(cSock, (DataHeader*)&ret);
		//send(cSock, (char*)&ret, sizeof(LogoutResult), 0);
	}
	break;
	default:
		std::cout << "未识别指令，error……,数据长度"<< header->dataLength <<"\n";
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
	// 广播通知其他客户端，有新客户端加入
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
		// 关闭全部socket
		closesocket(_clients[n]->sockfd());
		delete _clients[n]; // new 会把资源放到堆内存空间（堆内存空间就是电脑的内存卡非常大的空间，如果不是new的话，会自动放到栈内存空间，这个比较小一般只有一两M。其次new了之后必须要记得delete释放掉这部分资源。）
	}
	// 8.关闭套接字 closesocket
	closesocket(_sock);
	//清除Windows socket环境
	WSACleanup();
#else
	for (int n = 0; n < (int)_clients.size(); ++n)
	{
		// 关闭全部socket
		close(_clients[n]->sockfd());
		delete _clients[n];
	}
	close(_sock);
#endif
	_clients.clear(); // 清理一下vector容器 
}
