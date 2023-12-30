
//#include "baseClient.h"
#include "TcpClient.h"

bool g_bRun = true;
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

// 客户端数量
const int cCount = 10000;
// 线程数量
const int tCount = 4;
TcpClient* client[cCount];

std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;

void sendThread(int id)
{
	std::cout << "thread<"<< id <<">,start\n";
	//4个线程 ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id * c;

	for (int n = begin; n < end; ++n)
	{
		client[n] = new TcpClient();
	}
	for (int n = begin; n < end; ++n)
	{
		client[n]->Connect("127.0.0.1", 4567);
	}
	std::cout << "thread<"<< id <<">,Connect<begin="<< begin <<", end="<< end <<">\n";

	readyCount++;
	while (readyCount < tCount)
	{
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);

	}

	Login login[1];
	for (int n = 0; n < 1; n++)
	{
	strcpy(login[n].userName, "lyd");
	strcpy(login[n].passWord, "lydmm");

	}

	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int n = begin; n < end; ++n)
		{
			if (SOCKET_ERROR != client[n]->SendData(login, nLen))
			{
				// 发送成功；
				sendCount++;
			}
			client[n]->OnRun();
		}
	}

	for (int n = 0; n < cCount; ++n)
	{
		client[n]->Close();
		delete client[n];
	}
}


int main()
{


	std::thread t1(cmdInputThread);
	t1.detach();

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread, n + 1);
		t1.detach();
	}

	CELLTimestamp tTime;



	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			std::cout << "thread<"<< tCount <<">,clients<"<< cCount <<">,time<"<< t <<">,send<"<< (int)(sendCount/t) <<">\n";
			sendCount = 0;
			tTime.update();
		}
		std::chrono::milliseconds tt(1);
		std::this_thread::sleep_for(tt);
	}

	std::cout << "已退出\n";
	getchar();
	return 0;
}
