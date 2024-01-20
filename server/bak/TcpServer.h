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
#include "CELLTask.hpp"

typedef std::shared_ptr<DataHeader> DataHeaderPtr;
//typedef std::shared_ptr<LoginResult> LoginResultPtr;


//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240*5
#define SEND_BUFF_SZIE RECV_BUFF_SZIE
#endif // !RECV_BUFF_SZIE

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SZIE);
		_lastPos = 0;

		memset(_szSendBuf, 0, SEND_BUFF_SZIE);
		_lastSendPos = 0;
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
	int SendData(DataHeaderPtr& header)
	{
		int ret = SOCKET_ERROR;
		int nSendLen = header->dataLength;
		const char* pSendData = (const char*)header.get(); // ����ָ���get������ ��������ָ�������

		while (true)
		{
			if (nSendLen + _lastSendPos >= SEND_BUFF_SZIE)
			{
				//����ɿ��������ݳ���
				int nCopyLen = SEND_BUFF_SZIE - _lastSendPos;
				//��������
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//����ʣ������λ��
				pSendData += nCopyLen;
				//����ʣ�����ݳ���
				nSendLen -= nCopyLen;
				//��������
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SZIE, 0);
				//����β��λ������
				_lastSendPos = 0;
				//���ʹ���
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				//��Ҫ���͵����� ���������ͻ�����β��
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				//��������β��λ��
				_lastSendPos += nSendLen;
				break;
			}
		}
		return ret;
	}

private:
	SOCKET _sockfd;
	// �ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SZIE];
	// βָ��
	int _lastPos;

	//�ڶ������� ���ͻ�����
	char _szSendBuf[SEND_BUFF_SZIE];
	//���ͻ�����������β��λ��
	int _lastSendPos;
};

class CellServer;
typedef std::shared_ptr<ClientSocket> ClientSocketPtr;

// �ӿ��࣬�����¼��ӿ�
class INetEvent
{
public:
	// ���麯��
	// �ͻ��˼����¼�
	virtual void OnNetJoin(ClientSocketPtr& pClient) = 0;
	// �ͻ����뿪�¼�
	virtual void OnNetLeave(ClientSocketPtr& pClient) = 0;
	// �ͻ�����Ϣ�¼�
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header) = 0;
	// recv�¼�
	virtual void OnNetRecv(ClientSocketPtr& pClient) = 0;
};

class CellS2CTask : public CellTask
{
private:
	ClientSocketPtr _pClient;
	DataHeaderPtr _pHeader;

public:
	CellS2CTask(ClientSocketPtr& pClient, DataHeaderPtr& header)
	{
		_pClient = pClient;
		_pHeader = header;
	}

	void doTask()
	{
		_pClient->SendData(_pHeader);
		//delete _pHeader;
	}

};
typedef std::shared_ptr<CellS2CTask> CellS2CTaskPtr;

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
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				//delete iter.second;
			}
			// 8.�ر��׽��� closesocket
			closesocket(_sock);
			
#else
			for (auto iter : _clients)
			{
				// �ر�ȫ��socket
				close(iter.second->sockfd());
				delete iter.second;
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
			if(!_clientsBuff.empty())
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

			std::vector<ClientSocketPtr> temp;
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

	//�������� ����ճ�� ��ְ�
	int RecvData(ClientSocketPtr& pClient)
	{
		char* szRecv = pClient->szMsgBuf() + pClient->getLastPos();
		// 5.������Ϣ
		int nLen = (int)recv(pClient->sockfd(), szRecv, (RECV_BUFF_SZIE) - pClient->getLastPos(), 0);
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			// ����ʧ��
			//std::cout << "<�ͻ���: " << pClient->sockfd() << ">���ӶϿ�\n";
			return -1;
		}

		// ���ջ��������ݷ�����Ϣ������
		//memcpy(pClient->szMsgBuf() + pClient->getLastPos(), _szRecv, nLen);
		// ��Ϣ������βָ�����
		pClient->setLastPos(pClient->getLastPos() + nLen);

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
	virtual void OnNetMsg(ClientSocketPtr& pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this,pClient, header);
	}

	void addClient(ClientSocketPtr& pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	void addSendTask(ClientSocketPtr& pClient, DataHeaderPtr& header)
	{
		//auto task = new CellS2CTask(pClient, header);
		CellTaskPtr task = std::make_shared<CellS2CTask>(pClient, header);
		_taskServer.addTask(task);;
	}

private:
	SOCKET _sock;
	// ��ʽ�ͻ��˶���
	std::map<SOCKET, ClientSocketPtr> _clients;
	// ����ͻ��˶���
	std::vector<ClientSocketPtr> _clientsBuff;
	// ���������
	std::mutex _mutex;
	std::thread _thread;
	// �����¼�����
	INetEvent* _pNetEvent;

	// task������ ����
	CellTaskServer _taskServer;
};

typedef std::shared_ptr<CellServer> CellServerPtr;
class TcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//��Ϣ��������ڲ��ᴴ���߳�
	std::vector<CellServerPtr> _cellServers;
	CELLTimestamp _tTime; // ��ʱ

protected:
	// SOCKET recv����
	std::atomic_int _recvCount;
	//�յ���Ϣ����
	std::atomic_int _msgCount;
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

	void addClientToCellServer(ClientSocketPtr& pClient);

	void Start(int nCellServer);

	// �ر�socket
	void Close();

	// ��ѯ����IO
	bool OnRun();

	bool IsRun();

	//���㲢���ÿ���յ���������Ϣ
	void time4msg();

	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocketPtr& pClient);

	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(ClientSocketPtr& pClient);

	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header);

	// ��ȡ�����¼�
	virtual void OnNetRecv(ClientSocketPtr& pClient);

};

#endif


