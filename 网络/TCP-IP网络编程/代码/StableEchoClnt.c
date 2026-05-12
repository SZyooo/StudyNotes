#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFF_SIZE 1024

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN servAddr;
	char message[BUFF_SIZE];
	int strLen, readLen;
	if(argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup faild");
	}

	hSocket=socket(PF_INET, SOCK_STREAM, 0);
	if(hSocket==INVALID_SOCKET)
	{
		ErrorHandling("socket error");
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_addr.s_addr=inet_addr(argv[1]);
	servAddr.sin_port=htons(atoi(argv[2]));

	if(connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
	{
		ErrorHandling("connect error");
	}
	else
		puts("connected......");
	while(1)
	{
		fputs("Input message(Q or q to quit):", stdout);
		fgets(message, BUFF_SIZE, stdin);
		if(!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
		{
			break;
		}
		strLen=strlen(message);
		send(hSocket, message, strLen, 0);
		readLen=0;
		while(1)
		{
			readLen += recv(hSocket, &message[readLen], BUFF_SIZE-1-readLen, 0);
			if(readLen >= strLen)
				break;
		}
		message[strLen]=0;
		printf("Message from server: %s", message);
	}
	closesocket(hSocket);
	WSACleanup();
	return 0;
}
