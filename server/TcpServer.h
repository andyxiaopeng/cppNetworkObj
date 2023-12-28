#ifndef _TcpServer_
#define _TcpServer_

#ifdef _WIN32

	// 修改FD集合的大小，也就是select模型能管理的io数量
	#define FD_SETSIZE      2506

	#define  WIN32_LEAN_AND_MEAN // 这个宏尽量避免早期一些依赖库的引用
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <Windows.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h>		// linux是基于unix的，所以无论是linux还是unix都会引入一个unix的标准库
	#include <arpa/inet.h>	// linux网络编程相关的库
	#include <string.h>
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <iostream>
#include  <iomanip>
#include "dataType.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <map>
#include "CELLTimestamp.hpp"


//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* szMsgBuf()
	{
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
	//发送数据
	int SendData(DataHeader* header)
	{
		if (header)
		{
			return send(_sockfd, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

private:
	SOCKET _sockfd;
	// 第二缓存区 消息缓存区
	char _szMsgBuf[RECV_BUFF_SZIE * 5];
	// 尾指针
	int _lastPos;
};

// 接口类，网络事件接口
class INetEvent
{
public:
	// 纯虚函数
	// 客户端加入事件
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	// 客户端离开事件
	virtual void OnNetLeave(ClientSocket* pClient) = 0;
	// 客户端消息事件
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) = 0;
};

class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}

	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
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
			// //清除Windows socket环境
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
	}

	//是否工作中
	bool IsRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//处理网络消息
	//备份客户socket fd_set
	fd_set _fdRead_bak;
	//客户列表是否有变化
	bool _clients_change;
	SOCKET _maxSock;

	bool OnRun()
	{
		_clients_change = true;
		while (IsRun())
		{
			if(_clientsBuff.size() > 0)
			{
				// 从缓存客户端队列里面取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			if (_clients.empty())
			{
				//如果没有需要处理的客户端，就跳过
				// 线程休眠 1 毫秒
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			// 伯克利select
			// 创建三个socket集合 又称为 描述符
			// fd   ===  file describe   == 文件描述符
			fd_set fdRead;
			FD_ZERO(&fdRead);

			if (_clients_change)
			{
				_clients_change = false;
				_maxSock = _clients.begin()->second->sockfd(); // map元素的first方法和second方法分别是求取map元素的key和value

				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if ((_maxSock < iter.second->sockfd()))
					{
						_maxSock = iter.second->sockfd();
					}
				}
				// 循环检查了一次了之后吧fd集合的数据做个备份。后续没变化可以不再检查fd集合。
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}



			/// nfds 是一个整数值，是指fd_set集合中所有的描述符范围，而不是数量
			///	既是所有描述符最大值+1， 在windows中无所谓，传一个任意值都行
			//timeval t = { 0,10 }; // 两个0，代表没有需要等待时间 === 非阻塞。
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, nullptr); // select 是由内核提供的监听程序，客户端向服务端发送连接请求本质是一个读事件，而select就是监听这类读事件。
			// ！！！ps：若文件描述符fd（也就是socket）没有任何时间发生，那么在调用select后，fd在集合中的值会被置为0；
			// select()返回结果是fdRead\fdWrite\fdExp这三个集合发生事件的总数.
			if (ret < 0)
			{
				std::cout << "select任务 有错误！";
				Close();
				return false;
			}else if (ret == 0)
			{
				continue;
			}

#ifdef _WIN32
			for (int n = fdRead.fd_count - 1; n >= 0; --n)
			{
				auto iter = _clients.find(fdRead.fd_array[n]);
				if (iter != _clients.end())
				{
					if (-1 == RecvData(iter->second)) // prosessor 返回-1 表示该cSocket链接已经断开，所以需要从vector中将其删除
					{
						if (_pNetEvent)
						{
							_pNetEvent->OnNetLeave(iter->second); // 客户端离线通知
						}
						_clients_change = true;
						_clients.erase(iter->first); // 从容器擦除指定的元素
					}
				}
				else
				{
					std::cout << "error. if (iter != _clients.end())\n";
				}
			}
#else

			std::vector<ClientSocket*> temp;
			for (auto iter : _clients)
			{
				if (FD_ISSET(iter.second->sockfd(), &fdRead))
				{
					if (-1 == RecvData(iter.second)) // prosessor 返回-1 表示该cSocket链接已经断开，所以需要从vector中将其删除
					{
						if (_pNetEvent)
						{
							_pNetEvent->OnNetLeave(iter.second); // 客户端离线通知
						}
						_clients_change = false;
						temp.push_back(iter.second);
					}
				}
			}
			for (auto pClient : temp)
			{
				_clients.erase(pClient->sockfd());
				delete pClient;
			}
#endif
		}
	}

	//缓冲区
	char _szRecv[RECV_BUFF_SZIE] = {};
	//接收数据 处理粘包 拆分包
	int RecvData(ClientSocket* pClient)
	{
		// 5.接收信息
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0)
		{
			// 连接失败
			//std::cout << "<客户端: " << pClient->sockfd() << ">连接断开\n";
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
				OnNetMsg(pClient, header);
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

	//响应网络消息
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(pClient, header);
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

private:
	SOCKET _sock;
	// 正式客户端队列
	std::map<SOCKET,ClientSocket*> _clients;
	// 缓存客户端队列
	std::vector<ClientSocket*> _clientsBuff;
	// 缓存队列锁
	std::mutex _mutex;
	std::thread _thread;
	// 网络事件对象
	INetEvent* _pNetEvent;
};

class TcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//消息处理对象，内部会创建线程
	std::vector<CellServer*> _cellServers;
	CELLTimestamp _tTime; // 计时

protected:
	//收到消息计数
	std::atomic_int _recvCount;
	//客户端计数
	std::atomic_int _clientCount;

public:
	TcpServer();
	virtual ~TcpServer();

	// 初始化socket
	SOCKET InitSocket();
	// 绑定端口号
	int Bind(const char* ip, unsigned short port);
	// 监听端口号
	int Listen(int n);

	// 接收客户端连接
	SOCKET Accept();

	void addClientToCellServer(ClientSocket* pClient);

	void Start(int nCellServer);

	// 关闭socket
	void Close();

	// 查询网络IO
	bool OnRun();

	bool IsRun();

	//计算并输出每秒收到的网络消息
	void time4msg();

	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocket* pClient);

	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(ClientSocket* pClient);

	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header);

};

#endif


