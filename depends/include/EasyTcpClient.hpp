﻿#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#include"CELL.hpp"
#include"CELLNetWork.hpp"
#include"MessageHeader.hpp"
#include"CELLClient.hpp"
#include"CELLFDSet.hpp"

class EasyTcpClient
{
public:
	EasyTcpClient()
	{
		_isConnect = false;
	}
	
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	SOCKET InitSocket(int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE)
	{
		CELLNetWork::Init();

		if (_pClient)
		{
			CELLLog_Info("warning, initSocket close old socket<%d>...", (int)_pClient->sockfd());
			Close();
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			CELLLog_PError("create socket failed...");
		}
		else {
			CELLNetWork::make_reuseaddr(sock);
			//CELLLog_Info("create socket<%d> success...", (int)sock);
			_pClient = new CELLClient(sock, sendSize, recvSize);
		}
		return sock;
	}

	//连接服务器
	int Connect(const char* ip,unsigned short port)
	{
		if (!_pClient)
		{
			if (INVALID_SOCKET == InitSocket())
			{
				return SOCKET_ERROR;
			}
		}
		// 2 连接服务器 connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		//CELLLog_Info("<socket=%d> connecting <%s:%d>...", (int)_pClient->sockfd(), ip, port);
		int ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			CELLLog_PError("<socket=%d> connect <%s:%d> failed...", (int)_pClient->sockfd(), ip, port);
		}
		else {
			_isConnect = true;
			//CELLLog_Info("<socket=%d> connect <%s:%d> success...", (int)_pClient->sockfd(), ip, port);
		}
		return ret;
	}

	//关闭套节字closesocket
	void Close()
	{
		if (_pClient)
		{
			delete _pClient;
			_pClient = nullptr;
		}
		_isConnect = false;
	}

	//处理网络消息
	bool OnRun(int microseconds = 1)
	{
		if (isRun())
		{
			SOCKET _sock = _pClient->sockfd();


			_fdRead.zero();
			_fdRead.add(_sock);

			_fdWrite.zero();

			timeval t = { 0,microseconds };
			int ret = 0;
			if (_pClient->needWrite())
			{
				
				_fdWrite.add(_sock);
				ret = select(_sock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);
			}else {
				ret = select(_sock + 1, _fdRead.fdset(), nullptr, nullptr, &t);
			}

			if (ret < 0)
			{
				CELLLog_PError("<socket=%d>OnRun.select exit", (int)_sock);
				Close();
				return false;
			}

			if (_fdRead.has(_sock))
			{
				if (SOCKET_ERROR == RecvData(_sock))
				{
					CELLLog_Error("<socket=%d>OnRun.select RecvData exit", (int)_sock);
					Close();
					return false;
				}
			}

			if (_fdWrite.has(_sock))
			{
				if (SOCKET_ERROR == _pClient->SendDataReal())
				{
					CELLLog_Error("<socket=%d>OnRun.select SendDataReal exit", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _pClient && _isConnect;
	}

	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET cSock)
	{
		if (isRun())
		{
			//接收客户端数据
			int nLen = _pClient->RecvData();
			if (nLen > 0)
			{
				//循环 判断是否有消息需要处理
				while (_pClient->hasMsg())
				{
					//处理网络消息
					OnNetMsg(_pClient->front_msg());
					//移除消息队列（缓冲区）最前的一条数据
					_pClient->pop_front_msg();
				}
			}
			return nLen;
		}
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader* header) = 0;

	//发送数据
	int SendData(netmsg_DataHeader* header)
	{
		if(isRun())
			return _pClient->SendData(header);
		return SOCKET_ERROR;
	}

	int SendData(const char* pData, int len)
	{
		if (isRun())
			return _pClient->SendData(pData, len);
		return SOCKET_ERROR;
	}
protected:
	CELLFDSet _fdRead;
	CELLFDSet _fdWrite;
	CELLClient* _pClient = nullptr;
	bool _isConnect = false;
};

#endif