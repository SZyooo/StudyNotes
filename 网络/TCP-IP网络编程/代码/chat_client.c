#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
//#include <winsock2.h>

#define BUFF_SIZE 100
#define NAME_SIZE 20

char name[NAME_SIZE]="[DEFAULT]";
char msg[BUFF_SIZE];

unsigned WINAPI SendMsg(void* arg)
{
	SOCKET hSock=*((SOCKET*)arg);
	char nameMsg[NAME_SIZE + BUFF_SIZE];
	while(1)
	{
		fgets(msg, BUFF_SIZE, stdin);
		if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			closesocket(hSock);
			exit(0);
		}
		sprintf(nameMsg, "%s %s", name, msg);
		send(hSock, nameMsg, strlen(nameMsg), 0);
	}
	return 0;
}

unsigned WINAPI RecvMsg(void* arg)
{
	int hSock=*((SOCKET*)arg);
	char nameMsg[NAME_SIZE+BUFF_SIZE];
	int strLen;
	while (1) {
		strLen=recv(hSock, nameMsg, NAME_SIZE + BUFF_SIZE, 0);	
		if(strLen==-1)
		{
			return -1;
		}
		nameMsg[strLen]=0;
		fputs(nameMsg, stdout);
	}	
	return 0;
}

void HandleError(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSock;
	SOCKADDR_IN servAddr;
	HANDLE hSndThread, hClntThread;
	if(argc != 4)
	{
		printf("Usage : %s<IP> <port> <name> \n", argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
	{
		HandleError("WSAStartup failed");
	}
	sprintf(name, "[%s]", argv[3]);
	hSock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_addr.s_addr=inet_addr(argv[1]);
	servAddr.sin_port=htons(atoi(argv[2]));
	if(connect(hSock, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
		HandleError("connect failed");
	hSndThread=(HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);
	hClntThread=(HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);

	WaitForSingleObject(hSndThread, INFINITE);
	WaitForSingleObject(hClntThread, INFINITE);
	closesocket(hSock);
	WSACleanup();
	return 0;
}
