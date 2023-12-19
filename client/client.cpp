
//#include "baseClient.h"
#include "TcpClient.h"

int main()
{
	// startBaseClient();
	TcpClient tc;
	tc.InitSocket();
	tc.Connect("127.0.0.1", 4567);

	std::thread t1(cmdInputThread, &tc);
	t1.detach();

	while (tc.IsRun())
	{
		tc.OnRun();
	}

	tc.Close();
	std::cout << "ÒÑÍË³ö\n";
	getchar();
	return 0;
}
