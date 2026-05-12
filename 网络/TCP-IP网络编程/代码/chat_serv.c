#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
//#include <winsock2.h>

#define BUFF_SIZE 100
#define MAX_CLNT 256

int clntCnt=0;
SOCKET clntSocks[MAX_CLNT];
HANDLE hMutex;

void SendMsg(char* msg, int len);
unsigned WINAPI HandleClient(void* arg)
{
	SOCKET hClntSock=*((SOCKET*)arg);
	int strLen=0, i;
	char msg[BUFF_SIZE];

	while((strLen=recv(hClntSock, msg, sizeof(msg), 0))!=0)
	{
		SendMsg(msg, strLen);
	}
	WaitForSingleObject(hMutex, INFINITE);
	for(i = 0; i < clntCnt; ++ i)
	{
		if(hClntSock==clntSocks[i])
		{
			while(i++ < clntCnt-1)
				clntSocks[i]=clntSocks[i+1];
			break;
		}
	}
	clntCnt--;
	ReleaseMutex(hMutex);
	closesocket(hClntSock);
	return 0;
}

void SendMsg(char* msg, int len)
{
	int i;
	WaitForSingleObject(hMutex, INFINITE);
	for(i = 0; i < clntCnt; ++ i)
	{
		send(clntSocks[i], msg, len, 0);
	}
	ReleaseMutex(hMutex);
}

void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;
	int clntAddrSz;
	HANDLE hThread;
	if(argc != 2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
	{
		ErrorHandling("WSAStartup failed");
	}
	hMutex = CreateMutex(NULL, FALSE, NULL);
	hServSock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAddr.sin_port=htons(atoi(argv[1]));

	if(bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
		ErrorHandling("bind failed");
	if(listen(hServSock, 5)==SOCKET_ERROR)
		ErrorHandling("listen failed");

	while(1)
	{
		clntAddrSz=sizeof(clntAddr);
		hClntSock=accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSz);
		WaitForSingleObject(hMutex, INFINITE);
		clntSocks[clntCnt++]=hClntSock;
		ReleaseMutex(hMutex);

		hThread=(HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&hClntSock, 0, NULL);
		printf("Connected client IP: %s\n", inet_ntoa(clntAddr.sin_addr));
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}
