#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUFF_SIZE 1024

typedef struct
{
	SOCKET 	hClientSock;
	char 	buf[BUFF_SIZE];
	WSABUF 	wsaBuf;
}PER_IO_DATA, *LPPER_IO_DATA;

void CALLBACK WriteCompRoutine(DWORD dwErr, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags);
void CALLBACK ReadCompRoutine(DWORD dwErr, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo=(LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock=hbInfo->hClientSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD sentbytes;

	if(szRecvBytes==0)
	{
		closesocket(hSock);
		free(lpOverlapped->hEvent);
		free(lpOverlapped);
		puts("Client disconnected...");
	}
	else
	{
		bufInfo->len=szRecvBytes;
		WSASend(hSock, bufInfo, 1, &sentbytes, 0, lpOverlapped, WriteCompRoutine);
	}
}

void CALLBACK WriteCompRoutine(DWORD dwErr, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo=(LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock=hbInfo->hClientSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD recvBytes;
	int flagInfo=0;
	WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hListenSock, hRecvSock;
	SOCKADDR_IN listenAddr, recvAddr;
	LPWSAOVERLAPPED lpOverlapped;
	DWORD recvBytes;
	LPPER_IO_DATA hbInfo=NULL;
	int mode=1, recvAddrSz, flagInfo=0;
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
	ioctlsocket(hListenSock, FIONBIO, &mode);
	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family=AF_INET;
	listenAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	listenAddr.sin_port=htons(atoi(argv[1]));

	if(bind(hListenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr))==SOCKET_ERROR)
	{
		ErrorHandling("bind failed");
	}
	if(listen(hListenSock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen failed");
	}
	recvAddrSz=sizeof(recvAddr);
	while(1)
	{
		SleepEx(100, TRUE);
		hRecvSock=accept(hListenSock, (SOCKADDR*)&recvAddr, &recvAddrSz);
		if(hRecvSock==INVALID_SOCKET)
		{
			if(WSAGetLastError()==WSAEWOULDBLOCK)
				continue;
			else
				ErrorHandling("accept error");
		}
		puts("Client connected...");

		lpOverlapped=(LPOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		if (lpOverlapped == NULL)
			ErrorHandling("no memory");
		memset(lpOverlapped, 0, sizeof(WSAOVERLAPPED));
		hbInfo=(LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		if (hbInfo == NULL)
			ErrorHandling("no memory");
		hbInfo->hClientSock=(DWORD)hRecvSock;
		(hbInfo->wsaBuf).buf=hbInfo->buf;
		(hbInfo->wsaBuf).len = BUFF_SIZE;

		lpOverlapped->hEvent=(HANDLE)hbInfo;
		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
	}
	closesocket(hRecvSock);
	closesocket(hListenSock);
	WSACleanup();
	return 0;
}
