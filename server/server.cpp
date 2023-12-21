
#include "TcpServer.h"
int main()
{
	TcpServer ts = {};
	ts.InitSocket();
	ts.Bind(nullptr, 4567);
	ts.Listen(5);
	while (ts.IsRun())
	{
		ts.OnRun();
	}

	ts.Close();
	std::cout << "ÒÑÍË³ö\n";
	getchar();
	return 0;
}

// #include "baseServer.h"
//
// int main()
// {
// 	startBaseServer();
// }
