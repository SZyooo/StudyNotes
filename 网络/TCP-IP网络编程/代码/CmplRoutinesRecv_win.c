#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUFF_SIZE 1024

char buf[BUFF_SIZE];
int recvBytes;

WSABUF dataBuf;

void ErrorHandling(const char* err);
void CALLBACK CompRoutine(DWORD dwErr, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	if(dwErr != 0)
	{
		ErrorHandling("CompRoutine error");
	}
	else
	{
		recvBytes=szRecvBytes;
		printf("Received message : %s\n", buf);
	}
}

void ErrorHandling(const char* err)
{
	fputs(err, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hListenSock, hRecvSock;
	SOCKADDR_IN listenAddr, recvAddr;
	WSAOVERLAPPED overlapped;
	WSAEVENT evObj;

	int idx, recvAddrSz, flags=0;
	if(argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup failed");
	}

	hListenSock=WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family=AF_INET;
	listenAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	listenAddr.sin_port=htons(atoi(argv[1]));

	if(bind(hListenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr))==SOCKET_ERROR)
	{
		ErrorHandling("bind error");	
	}
	if(listen(hListenSock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen error");
	}
	recvAddrSz=sizeof(recvAddr);
	hRecvSock=accept(hListenSock, (SOCKADDR*)&recvAddr, &recvAddrSz);
	if(hRecvSock == INVALID_SOCKET)
	{
		ErrorHandling("listen() error");
	}

	memset(&overlapped, 0, sizeof(overlapped));
	
	dataBuf.buf=buf;
	dataBuf.len=BUFF_SIZE;


	if(WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR)
	{
		if(WSAGetLastError() ==WSA_IO_PENDING)
		{
			puts("Background data receive");
		}
	}
	//仅仅是用来调用WSAWaitForMultipleEvents进入alertable wait状态
	evObj=WSACreateEvent();
	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE);
	if(idx == WAIT_IO_COMPLETION)
	{
		puts("Overlapped I/O Complete");
	}
	else
		ErrorHandling("WSARecv error");
	WSACloseEvent(evObj);
	closesocket(hRecvSock);
	closesocket(hListenSock);
	WSACleanup();
	return 0;
}
