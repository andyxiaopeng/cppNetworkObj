#include "TcpClient.h"

TcpClient::TcpClient()
{
	_sock = INVALID_SOCKET;

	//接收缓冲区
	_szRecv[RECV_BUFF_SZIE] = {};
	//第二缓冲区 消息缓冲区
	_szMsgBuf[RECV_BUFF_SZIE * 10] = {};
	//消息缓冲区的数据尾部位置
	_lastPos = 0;

}

TcpClient::~TcpClient()
{
	Close();
}

int TcpClient::InitSocket()
{
	int ret = 0;
#ifdef _WIN32
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2); // winsocket的版本
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	// 简易客户端
	// 1.创建socket
	if (_sock != INVALID_SOCKET) // 防止初始化错误
	{
		std::cout << "已存在<socket = "<< _sock <<">，现将关闭旧连接……\n" ;
		Close();
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == INVALID_SOCKET)
	{// 创建失败
		std::cout << "创建socket失败\n";
		ret = 0;
	}
	else
	{
		std::cout << "创建socket成功\n";
		ret = 1;
	}
	return ret;
}

int TcpClient::Connect(const char* ip,const unsigned short port)
{
	int ret = 0;
	if (_sock == INVALID_SOCKET) // 防止未初始化socket就进行连接操作。
	{
		InitSocket();
	}

	// 2.连接服务器connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip); // 10.60.83.46
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif
	ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); // 连接服务器
	if (ret == SOCKET_ERROR)
	{// 连接失败
		std::cout << "连接服务器失败\n";
	}
	else
	{
		std::cout << "连接服务器成功\n";
	}
	return  ret;
}

void TcpClient::Close()
{
	if (_sock != INVALID_SOCKET) // 防止多次close
	{
		// 4.关闭套接字closesocket
#ifdef _WIN32
		closesocket(_sock);
		//清除Windows socket环境
		WSACleanup();
#else
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
		std::cout << "已退出\n";
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
			std::cout << "<socket:"<< _sock <<">select 有错误！";
			ret = false;
			return ret;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			ret = RecvData();// 消息处理
			if (ret == -1)
			{
				std::cout << "<socket:" << _sock << ">服务器断开\n";
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

int TcpClient::RecvData()
{
	int ret = 0;
	// 5.接收信息
	int nLen = (int)recv(_sock, _szRecv, RECV_BUFF_SZIE, 0);// 第三个参数是第二个参数的大小
	if (nLen <= 0)
	{
		// 连接失败
		std::cout << "与服务端<socket:"<< _sock <<">连接断开\n";
		ret = -1;
		return ret;
	}
	
	//将收取到的数据拷贝到消息缓冲区
	memcpy(_szMsgBuf + _lastPos, _szRecv, nLen); // 内存拷贝函数
	//消息缓冲区的数据尾部位置后移
	_lastPos += nLen;
	
	//判断消息缓冲区的数据长度大于消息头DataHeader长度
	while (_lastPos >= sizeof(DataHeader)) // 粘包
	{
		// 取消息头就能知道整个消息的长度
		DataHeader* header = (DataHeader*)_szMsgBuf;
		//判断消息缓冲区的数据长度大于消息长度
		if (_lastPos >= header->dataLength)
		{
			//消息缓冲区剩余未处理数据的长度
			int nSize = _lastPos - header->dataLength;
			
			//处理网络消息
			OnNetMsg(header);
			//将消息缓冲区剩余未处理数据前移
			memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
			// 将消息缓冲区尾指针前移
			_lastPos = nSize;
		}
		else
		{
			//消息缓冲区剩余数据不够一条完整消息
			break;
		}
	}

	ret = 0;
	return ret;
}

void TcpClient::OnNetMsg(DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
		{
			LoginResult* loginRet = (LoginResult*)header;
			//std::cout << "收到 <服务端：" << _sock << "> 消息： " << header->cmd << "   消息长度为：" << header->dataLength << "   result: " << loginRet->result << "\n";
			break;
		}
	case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutRet = (LogoutResult*)header;
			//std::cout << "收到 <服务端：" << _sock << "> 消息： " << header->cmd << "   消息长度为：" << header->dataLength << "   result: " << logoutRet->result << "\n";
			break;
		}
	case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* nUserRet = (NewUserJoin*)header;
			//std::cout << "收到 <服务端：" << _sock << "> 消息： " << header->cmd << "   消息长度为：" << header->dataLength << "   newUserSocketID: " << nUserRet->sock << "\n";
			break;
		}
	case CMD_ERROR:
		{
			std::cout << "<socket="<< _sock <<">收到服务端消息：CMD_ERROR,数据长度："<< header->dataLength <<"\n";
			break;
		}
	default:
		{
			std::cout << "<socket=" << _sock << ">收到未定义消息,数据长度：" << header->dataLength << "\n";
		}
	}
}

int TcpClient::SendData(DataHeader* header, int nLen)
{
	int ret = SOCKET_ERROR;
	if (IsRun() && header)
	{
		ret = send(_sock, (const char*)header, nLen, 0);
		if (SOCKET_ERROR == ret)
		{
			Close();
		}
	}
	return ret;
}



// void cmdInputThread(TcpClient* client)
// {
// 	while (true)
// 	{
// 		char msgBuf[1024] = {};
// 		std::cin >> msgBuf;
//
// 		if (strcmp("exit", msgBuf) == 0)
// 		{
// 			client->Close();
// 			std::cout << " cmdInputThread线程结束，客户端退出！\n";
// 			return;
// 		}
// 		else if (strcmp(msgBuf, "login") == 0)
// 		{
// 			// 3.发送命令
// 			Login login;
// #ifdef _WIN32
// 			strcpy_s(login.userName, "Andy");
// 			strcpy_s(login.passWord, "123456");
// #else
// 			strcpy(login.userName, "Andy");
// 			strcpy(login.passWord, "123456");
// #endif
//
// 			client->SendData(&login);
//
// 		}
// 		else if (strcmp(msgBuf, "logout") == 0)
// 		{
// 			Logout logout;
// #ifdef _WIN32
// 			strcpy_s(logout.userName, "Andy");
// #else
// 			strcpy(logout.userName, "Andy");
// #endif
// 			client->SendData(&logout);
// 		}
// 		else
// 		{
// 			std::cout << "不支持指令 请重新输入 \n";
// 		}
// 	}
// }
//
