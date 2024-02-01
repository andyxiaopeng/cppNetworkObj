#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"CELL.hpp"
#include"INetEvent.hpp"
#include"CELLClient.hpp"
#include"CELLSemaphore.hpp"
#include"CELLFDSet.hpp"

#include<vector>
#include<map>

//网络消息接收处理服务类
class CELLServer
{
public:
	CELLServer(int id)
	{
		_id = id;
		_pNetEvent = nullptr;
		_taskServer.serverId = id;
	}

	~CELLServer()
	{
		CELLLog_Info("CELLServer%d.~CELLServer exit begin", _id);
		Close();
		CELLLog_Info("CELLServer%d.~CELLServer exit end", _id);
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()
	{
		CELLLog_Info("CELLServer%d.Close begin", _id);
		_taskServer.Close();
		_thread.Close();
		CELLLog_Info("CELLServer%d.Close end", _id);
	}

	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					pClient->serverId = _id;
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				CELLThread::Sleep(1);
				//旧的时间戳
				_oldTime = CELLTime::getNowInMilliSec();
				continue;
			}

			CheckTime();
			if (!DoSelect())
			{
				pThread->Exit();
				break;
			}
			DoMsg();
		}
		CELLLog_Info("CELLServer%d.OnRun exit", _id);
	}

	bool DoSelect()
	{
		//计算可读集合
		if (_clients_change)
		{
			_clients_change = false;
			//清理集合
			_fdRead.zero();
			//将描述符（socket）加入集合
			_maxSock = _clients.begin()->second->sockfd();
			for (auto iter : _clients)
			{
				_fdRead.add(iter.second->sockfd());
				if (_maxSock < iter.second->sockfd())
				{
					_maxSock = iter.second->sockfd();
				}
			}
			_fdRead_bak.copy(_fdRead);
		}
		else {
			_fdRead.copy(_fdRead_bak);
		}
		//计算可写集合
		bool bNeedWrite = false;
		_fdWrite.zero();
		for (auto iter : _clients)
		{	//需要写数据的客户端,才加入fd_set检测是否可写
			if (iter.second->needWrite())
			{
				bNeedWrite = true;
				_fdWrite.add(iter.second->sockfd());
			}
		}

		///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
		///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
		timeval t{ 0,1 };
		int ret = 0;
		if (bNeedWrite)
		{
			ret = select(_maxSock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);
		}
		else {
			ret = select(_maxSock + 1, _fdRead.fdset(), nullptr, nullptr, &t);
		}
		if (ret < 0)
		{
            CELLLog_Error("CELLServer%d.OnRun.select Error exit:errno<%d>,errmsg<%s>", _id,errno,strerror(errno));
			return false;
		}
		else if (ret == 0)
		{
			return true;
		}
		ReadData();
		WriteData();
		return true;
	}

	void CheckTime()
	{
		//当前时间戳
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			//心跳检测
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent)
					_pNetEvent->OnNetLeave(iter->second);
				_clients_change = true;
				delete iter->second;
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}

			////定时发送检测
			//iter->second->checkSend(dt);

			iter++;
		}
	}
	void OnClientLeave(CELLClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pClient);
		_clients_change = true;
		delete pClient;
	}

	void WriteData()
	{
#ifdef _WIN32
		auto pfdset = _fdWrite.fdset();
		for (int n = 0; n < pfdset->fd_count; n++)
		{
			auto iter = _clients.find(pfdset->fd_array[n]);
			if (iter != _clients.end())
			{
				if (SOCKET_ERROR == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
		
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (iter->second->needWrite() && _fdWrite.has(iter->second->sockfd()))
			{
				if (SOCKET_ERROR == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}

	void ReadData()
	{
#ifdef _WIN32
			auto pfdset = _fdRead.fdset();
			for (int n = 0; n < pfdset->fd_count; n++)
			{
				auto iter = _clients.find(pfdset->fd_array[n]);
				if (iter != _clients.end())
				{
					if (SOCKET_ERROR == RecvData(iter->second))
					{
						OnClientLeave(iter->second);
						_clients.erase(iter);
					}
				}
			}
#else
			for (auto iter = _clients.begin(); iter != _clients.end(); )
			{
				if (_fdRead.has(iter->second->sockfd()))
				{
					if (SOCKET_ERROR == RecvData(iter->second))
					{
						OnClientLeave(iter->second);
						auto iterOld = iter;
						iter++;
						_clients.erase(iterOld);
						continue;
					}
				}
				iter++;
			}
#endif
	}

	void DoMsg()
	{
		CELLClient* pClient = nullptr;
		for (auto itr : _clients)
		{
			pClient = itr.second;
			//循环 判断是否有消息需要处理
			while (pClient->hasMsg())
			{
				//处理网络消息
				OnNetMsg(pClient, pClient->front_msg());
				//移除消息队列（缓冲区）最前的一条数据
				pClient->pop_front_msg();
			}
		}
	}

	//接收数据 处理粘包 拆分包
	int RecvData(CELLClient* pClient)
	{
		//接收客户端数据
		int nLen = pClient->RecvData();
		//触发<接收到网络数据>事件
		_pNetEvent->OnNetRecv(pClient);
		return nLen;
	}

	//响应网络消息
	virtual void OnNetMsg(CELLClient* pClient, netmsg_DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(CELLClient* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_taskServer.Start();
		_thread.Start(
			//onCreate
			nullptr,
			//onRun
			[this](CELLThread* pThread) {
				OnRun(pThread);
			},
			//onDestory
			[this](CELLThread* pThread) {
				ClearClients();
			}
		);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	//void addSendTask(CELLClient* pClient, netmsg_DataHeader* header)
	//{
	//	_taskServer.addTask([pClient, header]() {
	//		pClient->SendData(header);
	//		delete header;
	//	});
	//}
private:
	void ClearClients()
	{
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();

		for (auto iter : _clientsBuff)
		{
			delete iter;
		}
		_clientsBuff.clear();
	}
private:
	//正式客户队列
	std::map<SOCKET, CELLClient*> _clients;
	//缓冲客户队列
	std::vector<CELLClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//网络事件对象
	INetEvent* _pNetEvent;
	//
	CELLTaskServer _taskServer;
	//伯克利套接字 BSD socket
	//描述符（socket） 集合
	CELLFDSet _fdRead;
	CELLFDSet _fdWrite;
	//备份客户socket fd_set
	CELLFDSet _fdRead_bak;
	//
	SOCKET _maxSock;
	//旧的时间戳
	time_t _oldTime = CELLTime::getNowInMilliSec();
	//
	CELLThread _thread;
	//
	int _id = -1;
	//客户列表是否有变化
	bool _clients_change = true;
};

#endif // !_CELL_SERVER_HPP_
