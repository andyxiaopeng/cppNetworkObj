#define WIN32_LEAN_AND_MEAN

#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#include<mswsock.h>

#include<stdio.h>

//��AcceptEx���������ڴ��У�����Ч�ʸ���
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

//���ݻ������ռ��С
#define DATA_BUFF_SIZE 1024
struct IO_DATA_BASE
{
	//�ص���
	OVERLAPPED overlapped;
	//
	SOCKET sockfd;
	//���ݻ�����
	char buffer[DATA_BUFF_SIZE];
	//ʵ�ʻ��������ݳ���
	int length;
	//��������
	IO_TYPE iotype;
};
//��IOCPͶ�ݽ������ӵ�����
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
//��IOCPͶ�ݽ������ݵ�����
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
//��IOCPͶ�ݷ������ݵ�����
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
//-- ��Socket API��������TCP�����
//-- IOCP Server��������
int main()
{
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------//
	// 1 ����һ��socket
	// ��ʹ��socket���������׽���ʱ����Ĭ������WSA_FLAG_OVERLAPPED��־
	//////
	// ע������Ҳ������ WSASocket��������socket
	// ���һ��������Ҫ����Ϊ�ص���־��WSA_FLAG_OVERLAPPED��
	// SOCKET sockServer = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//////
	SOCKET sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 2.1 ���ö���IP��˿���Ϣ 
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsigned short
	_sin.sin_addr.s_addr = INADDR_ANY;
	// 2.2 ��sockaddr��ServerSocket
	if (SOCKET_ERROR == bind(sockServer, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("����,������˿�ʧ��...\n");
	}
	else {
		printf("������˿ڳɹ�...\n");
	}

	// 3 ����ServerSocket
	if (SOCKET_ERROR == listen(sockServer, 64))
	{
		printf("����,��������˿�ʧ��...\n");
	}
	else {
		printf("��������˿ڳɹ�...\n");
	}
	//-------IOCP Begin-------//

	//4 ������ɶ˿�IOCP
	HANDLE _completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == _completionPort)
	{
		printf("CreateIoCompletionPort failed with error %d\n", GetLastError());
		return -1;
	}
	//5 ����IOCP��ServerSocket
	//��ɼ�
	auto ret = CreateIoCompletionPort((HANDLE)sockServer, _completionPort, (ULONG_PTR)sockServer, 0);
	if (!ret)
	{
		printf("����IOCP��ServerSocketʧ��,������=%d\n", GetLastError());
		return -1;
	}

	//6 ��IOCPͶ�ݽ������ӵ�����
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
				printf("�ر� sockfd=%d\n", pIOData->sockfd);
				closesocket(pIOData->sockfd);
				continue;
			}
			printf("GetQueuedCompletionStatus failed with error %d\n", err);
			break;
		}
		//7.1 �������� ���
		if (IO_TYPE::ACCEPT == pIOData->iotype)
		{
			printf("�¿ͻ��˼��� sockfd=%d\n", pIOData->sockfd);
			//7.2 ����IOCP��ClientSocket
			auto ret = CreateIoCompletionPort((HANDLE)pIOData->sockfd, _completionPort, (ULONG_PTR)pIOData->sockfd, 0);
			if (!ret)
			{
				printf("����IOCP��ClientSocket=%dʧ��\n", pIOData->sockfd);
				closesocket(pIOData->sockfd);
				continue;
			}
			//7.3 ��IOCPͶ�ݽ�����������
			postRecv(pIOData);
		}
		//8.1 �������� ��� Completion
		else if (IO_TYPE::RECV == pIOData->iotype)
		{
			if (bytesTrans <= 0)
			{//�ͻ��˶Ͽ�����
				printf("�ر� sockfd=%d, RECV bytesTrans=%d\n", pIOData->sockfd, bytesTrans);
				closesocket(pIOData->sockfd);
				continue;
			}
			printf("�յ�����: sockfd=%d, bytesTrans=%d msgCount=%d\n", pIOData->sockfd, bytesTrans, ++msgCount);
			pIOData->length = bytesTrans;
			//8.2 ��IOCPͶ�ݷ�����������
			postSend(pIOData);
		}
		//9.1 �������� ��� Completion
		else if (IO_TYPE::SEND == pIOData->iotype)
		{
			if (bytesTrans <= 0)
			{//�ͻ��˶Ͽ�����
				printf("�ر� sockfd=%d, SEND bytesTrans=%d\n", pIOData->sockfd, bytesTrans);
				closesocket(pIOData->sockfd);
				continue;
			}
			printf("��������: sockfd=%d, bytesTrans=%d msgCount=%d\n", pIOData->sockfd, bytesTrans, msgCount);
			//9.2 ��IOCPͶ�ݽ�����������
			postRecv(pIOData);
		}
		else {
			printf("δ������Ϊ sockfd=%d", sock);
		}
	}

	//------------//
	//10.1 �ر�ClientSocket
	for (int n = 0; n < nClient; n++)
	{
		closesocket(ioData[n].sockfd);
	}
	//10.2 �ر�ServerSocket
	closesocket(sockServer);
	//10.3 �ر���ɶ˿�
	CloseHandle(_completionPort);
	//���Windows socket����
	WSACleanup();
	return 0;
}