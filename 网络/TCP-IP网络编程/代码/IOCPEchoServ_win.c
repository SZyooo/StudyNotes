#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>

#define BUFF_SIZE 100
#define READ 	3
#define WRITE 	5

typedef struct{
	SOCKET 		hClientSock;       
	SOCKADDR_IN 	clientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct
{
	OVERLAPPED	overlapped;
	WSABUF		wsaBuf;
	char		buffer[BUFF_SIZE];
	int 		rwMode;
}PER_IO_DATA, *LPPER_IO_DATA;

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO)
{
	HANDLE				hComPort=(HANDLE)CompletionPortIO;
	SOCKET 				sock;
	DWORD 				bytesTrans;
	LPPER_HANDLE_DATA 	handleInfo;
	LPPER_IO_DATA		ioInfo;
	DWORD				flags = 0;
	LPOVERLAPPED 		lpOverlapped;
	while(1)
	{
		if (!GetQueuedCompletionStatus(hComPort, &bytesTrans, (PULONG_PTR)&handleInfo, (LPOVERLAPPED*)&lpOverlapped, INFINITE))
		{
			return 0;
		}
		ioInfo = (LPPER_IO_DATA)lpOverlapped;
		sock=handleInfo->hClientSock;
		if(ioInfo->rwMode==READ)
		{
			puts("message received!");
			if (bytesTrans == 0)
			{
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				continue;
			}
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = WRITE;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			if(ioInfo==NULL)
			{
				exit(-1);
			}
			memset(ioInfo, 0, sizeof(PER_IO_DATA));
			ioInfo->wsaBuf.len = BUFF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else
		{
			puts("message sent!");
			free(ioInfo);
		}
	}
	return 0;
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc ,char* argv[])
{
	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;
	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	int recvBytes, i, flags=0;
	if(WSAStartup(MAKEWORD(2 ,2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() failed");
	}
	hComPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(hComPort == NULL)
	{
		ErrorHandling("CreateIoCompletionPort() error");
	}
	GetSystemInfo(&sysInfo);
	for(i=0;i<sysInfo.dwNumberOfProcessors;i++)
	{
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);
	}	
	hServSock=WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAddr.sin_port=htons(atoi(argv[1]));

	bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	listen(hServSock, 5);

	while(1)
	{
		SOCKET hClientSock;
		SOCKADDR_IN clientAddr;
		int addrLen=sizeof(clientAddr);

		hClientSock=accept(hServSock, (SOCKADDR*)& clientAddr, &addrLen);
		if(hClientSock == INVALID_SOCKET)
		{
			ErrorHandling("accept() error");
		}
		handleInfo=(LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		if(handleInfo==NULL)
		{
			ErrorHandling("malloc() error");
			exit(-1);
		}
		handleInfo->hClientSock = hClientSock;
		memcpy(&(handleInfo->clientAddr), &clientAddr, addrLen);
		hComPort = CreateIoCompletionPort((HANDLE)hClientSock, hComPort, (ULONG_PTR)handleInfo, 0);
		if(hComPort == NULL)
		{
			ErrorHandling("CreateIoCompletionPort() error");
			exit(-1);
		}
		ioInfo=(LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		if(ioInfo==NULL)
		{
			ErrorHandling("malloc() error");
			exit(-1);
		}
		memset(ioInfo, 0, sizeof(PER_IO_DATA));
		ioInfo->wsaBuf.len = BUFF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode=READ;
		if(WSARecv(hClientSock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL) == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSA_IO_PENDING)
			{
				fprintf(stderr, "WSARecv() error: %d\n", WSAGetLastError());
				exit(-1);
			}
		}
	}
	return 0;
}
