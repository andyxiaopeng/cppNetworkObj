#include "Alloctor.h"
#include "TcpServer.h"


// 控制程序运行的bool
bool g_bRun = true;
void cmdInputThread()
{
	while (true)
	{
		char msgBuf[256] = {};
		std::cin >> msgBuf;

		if (strcmp("exit", msgBuf) == 0)
		{
			g_bRun = false;
			std::cout << " cmdInputThread线程结束，客户端退出！\n";
			return;
		}
		else
		{
			std::cout << "不支持指令 请重新输入 \n";
		}
	}
}

class MyServer : public TcpServer
{
public:
	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocketPtr& pClient)
	{
		TcpServer::OnNetJoin(pClient);
		//printf("client<%d> join\n", pClient->sockfd());
	}

	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(ClientSocketPtr& pClient)
	{
		TcpServer::OnNetLeave(pClient);
		//printf("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header)
	{
		TcpServer::OnNetMsg(pCellServer,pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login* login = (Login*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			//LoginResult ret;
			//pClient->SendData(&ret);
			//LoginResult* ret = new LoginResult();
			DataHeaderPtr ret = std::make_shared<LoginResult>();
			pCellServer->addSendTask(pClient, ret);
			//pCellServer->addSendTask(pClient, (DataHeaderPtr)ret);
			//pCellServer->addSendTask(pClient, (DataHeaderPtr)std::make_shared<LoginResult>());
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			std::cout << "<socket="<< pClient->sockfd() <<">收到未定义消息,数据长度："<< header->dataLength <<"\n";
		}
		break;
		}
	}
};


int main()
{
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);


	std::thread t1(cmdInputThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
	}

	server.Close();
	std::cout << "已退出\n";
	getchar();
	return 0;
}
