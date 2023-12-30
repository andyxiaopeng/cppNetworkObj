
#include "TcpServer.h"


// ���Ƴ������е�bool
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
			std::cout << " cmdInputThread�߳̽������ͻ����˳���\n";
			return;
		}
		else
		{
			std::cout << "��֧��ָ�� ���������� \n";
		}
	}
}

class MyServer : public TcpServer
{
public:
	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		TcpServer::OnNetJoin(pClient);
		//printf("client<%d> join\n", pClient->sockfd());
	}

	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		TcpServer::OnNetLeave(pClient);
		//printf("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header)
	{
		TcpServer::OnNetMsg(pCellServer,pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login* login = (Login*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LoginResult ret;
			//pClient->SendData(&ret);
			LoginResult* ret = new LoginResult();
			pCellServer->addSendTask(pClient, ret);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			std::cout << "<socket="<< pClient->sockfd() <<">�յ�δ������Ϣ,���ݳ��ȣ�"<< header->dataLength <<"\n";
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
	std::cout << "���˳�\n";
	getchar();
	return 0;
}
