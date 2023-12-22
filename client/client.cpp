
//#include "baseClient.h"
#include "TcpClient.h"

bool g_bRun;
void cmdInputThread()
{
	while (true)
	{
		char msgBuf[1024] = {};
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

int main()
{
	g_bRun = true;

	const int cCount = 10;
	TcpClient* client[cCount];
	
	for (int n = 0;n< cCount;++n)
	{
		client[n] = new TcpClient();
	}
	for (int n = 0; n < cCount; ++n)
	{
		client[n]->Connect("127.0.0.1", 4567);
	}

	// TcpClient tc;
	// tc.InitSocket();
	// tc.Connect("127.0.0.1", 4567); //119.23.212.84

	std::thread t1(cmdInputThread);
	t1.detach();

	Login login;
#ifdef _WIN32
	strcpy_s(login.userName, "Andy");
	strcpy_s(login.passWord, "123456");
#else
	strcpy(login.userName, "Andy");
	strcpy(login.passWord, "123456");
#endif


	while (g_bRun)
	{
		// tc.OnRun(); // 客户端收数据
		// tc.SendData(&login); // 客户端发数据

		for (int n = 0;n < cCount;++n)
		{
			client[n]->SendData(&login);
			client[n]->OnRun();
		}
	}

	for (int n = 0; n < cCount; ++n)
	{
		client[n]->Close();
		delete client[n];
	}

	// tc.Close();
	std::cout << "已退出\n";
	getchar();
	return 0;
}
