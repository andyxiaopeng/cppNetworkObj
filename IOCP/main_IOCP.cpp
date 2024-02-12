#define WIN32_LEAN_AND_MEAN

#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#include<mswsock.h>

#include<stdio.h>

//将AcceptEx函数加载内存中，调用效率更高
LPFN_ACCEPTEX lpfnAcceptEx = NULL;
void loadAcceptEx(SOCKET ListenSocket)
{
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	int iResult = WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&dwBytes, NULL, NULL);

	if (iResult == SOCKET_ERROR) {
		printf("WSAIoctl failed with error: %u\n", WSAGetLastError());
	}
}

#define nClient 10
enum IO_TYPE
{
	ACCEPT = 10,
	RECV,
	SEND
};

//数据缓冲区空间大小
#define DATA_BUFF_SIZE 1024
struct IO_DATA_BASE
{
	//重叠体
	OVERLAPPED overlapped;
	//
	SOCKET sockfd;
	//数据缓冲区
	char buffer[DATA_BUFF_SIZE];
	//实际缓冲区数据长度
	int length;
	//操作类型
	IO_TYPE iotype;
};
//向IOCP投递接受连接的任务
void postAccept(SOCKET sockServer, IO_DATA_BASE* pIO_DATA)
{
	pIO_DATA->iotype = IO_TYPE::ACCEPT;
	pIO_DATA->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (FALSE == lpfnAcceptEx(sockServer
		, pIO_DATA->sockfd
		, pIO_DATA->buffer
		, 0
		, sizeof(sockaddr_in) + 16
		, sizeof(sockaddr_in) + 16
		, NULL
		, &pIO_DATA->overlapped
	))
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			printf("AcceptEx failed with error %d\n", err);
			return;
		}
	}
}
//向IOCP投递接收数据的任务
void postRecv(IO_DATA_BASE* pIO_DATA)
{
	pIO_DATA->iotype = IO_TYPE::RECV;
	WSABUF wsBuff = {};
	wsBuff.buf = pIO_DATA->buffer;
	wsBuff.len = DATA_BUFF_SIZE;
	DWORD flags = 0;
	ZeroMemory(&pIO_DATA->overlapped, sizeof(OVERLAPPED));

	if (SOCKET_ERROR == WSARecv(pIO_DATA->sockfd, &wsBuff, 1, NULL, &flags, &pIO_DATA->overlapped, NULL))
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			printf("WSARecv failed with error %d\n", err);
			return;
		}
	}
}
//向IOCP投递发送数据的任务
void postSend(IO_DATA_BASE* pIO_DATA)
{
	pIO_DATA->iotype = IO_TYPE::SEND;
	WSABUF wsBuff = {};
	wsBuff.buf = pIO_DATA->buffer;
	wsBuff.len = pIO_DATA->length;
	DWORD flags = 0;
	ZeroMemory(&pIO_DATA->overlapped, sizeof(OVERLAPPED));

	if (SOCKET_ERROR == WSASend(pIO_DATA->sockfd, &wsBuff, 1, NULL, flags, &pIO_DATA->overlapped, NULL))
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			printf("WSASend failed with error %d\n", err);
			return;
		}
	}
}
//-- 用Socket API建立简易TCP服务端
//-- IOCP Server基础流程
int main()
{
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------//
	// 1 建立一个socket
	// 当使用socket函数创建套接字时，会默认设置WSA_FLAG_OVERLAPPED标志
	//////
	// 注意这里也可以用 WSASocket函数创建socket
	// 最后一个参数需要设置为重叠标志（WSA_FLAG_OVERLAPPED）
	// SOCKET sockServer = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//////
	SOCKET sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 2.1 设置对外IP与端口信息 
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsigned short
	_sin.sin_addr.s_addr = INADDR_ANY;
	// 2.2 绑定sockaddr与ServerSocket
	if (SOCKET_ERROR == bind(sockServer, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("错误,绑定网络端口失败...\n");
	}
	else {
		printf("绑定网络端口成功...\n");
	}

	// 3 监听ServerSocket
	if (SOCKET_ERROR == listen(sockServer, 64))
	{
		printf("错误,监听网络端口失败...\n");
	}
	else {
		printf("监听网络端口成功...\n");
	}
	//-------IOCP Begin-------//

	//4 创建完成端口IOCP
	HANDLE _completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == _completionPort)
	{
		printf("CreateIoCompletionPort failed with error %d\n", GetLastError());
		return -1;
	}
	//5 关联IOCP与ServerSocket
	//完成键
	auto ret = CreateIoCompletionPort((HANDLE)sockServer, _completionPort, (ULONG_PTR)sockServer, 0);
	if (!ret)
	{
		printf("关联IOCP与ServerSocket失败,错误码=%d\n", GetLastError());
		return -1;
	}

	//6 向IOCP投递接受连接的任务
	loadAcceptEx(sockServer);
	IO_DATA_BASE ioData[nClient] = {};
	for (int n = 0; n < nClient; n++)
	{
		postAccept(sockServer, &ioData[n]);
	}

	int msgCount = 0;
	while (true)
	{
		DWORD bytesTrans = 0;
		SOCKET sock = INVALID_SOCKET;
		IO_DATA_BASE* pIOData;

		if (FALSE == GetQueuedCompletionStatus(_completionPort, &bytesTrans, (PULONG_PTR)&sock, (LPOVERLAPPED*)&pIOData, 1))
		{
			int err = GetLastError();
			if (WAIT_TIMEOUT == err)
			{
				continue;
			}
			if (ERROR_NETNAME_DELETED == err)
			{
				printf("关闭 sockfd=%d\n", pIOData->sockfd);
				closesocket(pIOData->sockfd);
				continue;
			}
			printf("GetQueuedCompletionStatus failed with error %d\n", err);
			break;
		}
		//7.1 接受链接 完成
		if (IO_TYPE::ACCEPT == pIOData->iotype)
		{
			printf("新客户端加入 sockfd=%d\n", pIOData->sockfd);
			//7.2 关联IOCP与ClientSocket
			auto ret = CreateIoCompletionPort((HANDLE)pIOData->sockfd, _completionPort, (ULONG_PTR)pIOData->sockfd, 0);
			if (!ret)
			{
				printf("关联IOCP与ClientSocket=%d失败\n", pIOData->sockfd);
				closesocket(pIOData->sockfd);
				continue;
			}
			//7.3 向IOCP投递接收数据任务
			postRecv(pIOData);
		}
		//8.1 接收数据 完成 Completion
		else if (IO_TYPE::RECV == pIOData->iotype)
		{
			if (bytesTrans <= 0)
			{//客户端断开处理
				printf("关闭 sockfd=%d, RECV bytesTrans=%d\n", pIOData->sockfd, bytesTrans);
				closesocket(pIOData->sockfd);
				continue;
			}
			printf("收到数据: sockfd=%d, bytesTrans=%d msgCount=%d\n", pIOData->sockfd, bytesTrans, ++msgCount);
			pIOData->length = bytesTrans;
			//8.2 向IOCP投递发送数据任务
			postSend(pIOData);
		}
		//9.1 发送数据 完成 Completion
		else if (IO_TYPE::SEND == pIOData->iotype)
		{
			if (bytesTrans <= 0)
			{//客户端断开处理
				printf("关闭 sockfd=%d, SEND bytesTrans=%d\n", pIOData->sockfd, bytesTrans);
				closesocket(pIOData->sockfd);
				continue;
			}
			printf("发送数据: sockfd=%d, bytesTrans=%d msgCount=%d\n", pIOData->sockfd, bytesTrans, msgCount);
			//9.2 向IOCP投递接收数据任务
			postRecv(pIOData);
		}
		else {
			printf("未定义行为 sockfd=%d", sock);
		}
	}

	//------------//
	//10.1 关闭ClientSocket
	for (int n = 0; n < nClient; n++)
	{
		closesocket(ioData[n].sockfd);
	}
	//10.2 关闭ServerSocket
	closesocket(sockServer);
	//10.3 关闭完成端口
	CloseHandle(_completionPort);
	//清除Windows socket环境
	WSACleanup();
	return 0;
}