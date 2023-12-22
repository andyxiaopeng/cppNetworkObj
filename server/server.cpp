
#include "TcpServer.h"


// 控制程序运行的bool
bool g_bRun;
void cmdInputThread()
{
	while (true)
	{
		char msgBuf[4096] = {};
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
	TcpServer ts = {};
	ts.InitSocket();
	ts.Bind(nullptr, 4567);
	ts.Listen(5);

	std::thread t1(cmdInputThread);
	t1.detach();

	while (g_bRun)
	{
		ts.OnRun();
	}

	ts.Close();
	std::cout << "已退出\n";
	getchar();
	return 0;
}

// #include "baseServer.h"
//
// int main()
// {
// 	startBaseServer();
// }
