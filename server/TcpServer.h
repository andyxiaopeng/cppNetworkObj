#ifndef _TcpServer_
#define _TcpServer_

#ifdef _WIN32

	// �޸�FD���ϵĴ�С��Ҳ����selectģ���ܹ����io����
	#define FD_SETSIZE      2506

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
#include  <iomanip>
#include "dataType.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <map>
#include "CELLTimestamp.hpp"


//��������С��Ԫ��С
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
	//��������
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
	// �ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SZIE * 5];
	// βָ��
	int _lastPos;
};

// �ӿ��࣬�����¼��ӿ�
class INetEvent
{
public:
	// ���麯��
	// �ͻ��˼����¼�
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	// �ͻ����뿪�¼�
	virtual void OnNetLeave(ClientSocket* pClient) = 0;
	// �ͻ�����Ϣ�¼�
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

	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
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
			// //���Windows socket����
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
	}

	//�Ƿ�����
	bool IsRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//����������Ϣ
	//���ݿͻ�socket fd_set
	fd_set _fdRead_bak;
	//�ͻ��б��Ƿ��б仯
	bool _clients_change;
	SOCKET _maxSock;

	bool OnRun()
	{
		_clients_change = true;
		while (IsRun())
		{
			if(_clientsBuff.size() > 0)
			{
				// �ӻ���ͻ��˶�������ȡ���ͻ�����
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
				//���û����Ҫ����Ŀͻ��ˣ�������
				// �߳����� 1 ����
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			// ������select
			// ��������socket���� �ֳ�Ϊ ������
			// fd   ===  file describe   == �ļ�������
			fd_set fdRead;
			FD_ZERO(&fdRead);

			if (_clients_change)
			{
				_clients_change = false;
				_maxSock = _clients.begin()->second->sockfd(); // mapԪ�ص�first������second�����ֱ�����ȡmapԪ�ص�key��value

				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if ((_maxSock < iter.second->sockfd()))
					{
						_maxSock = iter.second->sockfd();
					}
				}
				// ѭ�������һ����֮���fd���ϵ������������ݡ�����û�仯���Բ��ټ��fd���ϡ�
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}



			/// nfds ��һ������ֵ����ָfd_set���������е���������Χ������������
			///	�����������������ֵ+1�� ��windows������ν����һ������ֵ����
			//timeval t = { 0,10 }; // ����0������û����Ҫ�ȴ�ʱ�� === ��������
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, nullptr); // select �����ں��ṩ�ļ������򣬿ͻ��������˷���������������һ�����¼�����select���Ǽ���������¼���
			// ������ps�����ļ�������fd��Ҳ����socket��û���κ�ʱ�䷢������ô�ڵ���select��fd�ڼ����е�ֵ�ᱻ��Ϊ0��
			// select()���ؽ����fdRead\fdWrite\fdExp���������Ϸ����¼�������.
			if (ret < 0)
			{
				std::cout << "select���� �д���";
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
					if (-1 == RecvData(iter->second)) // prosessor ����-1 ��ʾ��cSocket�����Ѿ��Ͽ���������Ҫ��vector�н���ɾ��
					{
						if (_pNetEvent)
						{
							_pNetEvent->OnNetLeave(iter->second); // �ͻ�������֪ͨ
						}
						_clients_change = true;
						_clients.erase(iter->first); // ����������ָ����Ԫ��
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
					if (-1 == RecvData(iter.second)) // prosessor ����-1 ��ʾ��cSocket�����Ѿ��Ͽ���������Ҫ��vector�н���ɾ��
					{
						if (_pNetEvent)
						{
							_pNetEvent->OnNetLeave(iter.second); // �ͻ�������֪ͨ
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

	//������
	char _szRecv[RECV_BUFF_SZIE] = {};
	//�������� ����ճ�� ��ְ�
	int RecvData(ClientSocket* pClient)
	{
		// 5.������Ϣ
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0)
		{
			// ����ʧ��
			//std::cout << "<�ͻ���: " << pClient->sockfd() << ">���ӶϿ�\n";
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
				OnNetMsg(pClient, header);
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

	//��Ӧ������Ϣ
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
	// ��ʽ�ͻ��˶���
	std::map<SOCKET,ClientSocket*> _clients;
	// ����ͻ��˶���
	std::vector<ClientSocket*> _clientsBuff;
	// ���������
	std::mutex _mutex;
	std::thread _thread;
	// �����¼�����
	INetEvent* _pNetEvent;
};

class TcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//��Ϣ��������ڲ��ᴴ���߳�
	std::vector<CellServer*> _cellServers;
	CELLTimestamp _tTime; // ��ʱ

protected:
	//�յ���Ϣ����
	std::atomic_int _recvCount;
	//�ͻ��˼���
	std::atomic_int _clientCount;

public:
	TcpServer();
	virtual ~TcpServer();

	// ��ʼ��socket
	SOCKET InitSocket();
	// �󶨶˿ں�
	int Bind(const char* ip, unsigned short port);
	// �����˿ں�
	int Listen(int n);

	// ���տͻ�������
	SOCKET Accept();

	void addClientToCellServer(ClientSocket* pClient);

	void Start(int nCellServer);

	// �ر�socket
	void Close();

	// ��ѯ����IO
	bool OnRun();

	bool IsRun();

	//���㲢���ÿ���յ���������Ϣ
	void time4msg();

	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocket* pClient);

	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(ClientSocket* pClient);

	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header);

};

#endif


