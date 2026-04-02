
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFF_SZ 30

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(-1);
}


int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET sock;
	char message[BUFF_SZ];
	int str_len;
	SOCKADDR_IN servAddr;
	if(argc != 3)
	{
		printf("Usage : %s <ip> <port> \n", argv[0]);
		exit(-1);
	}

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() failed");
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET)
	{
		ErrorHandling("socket failed");
	}

	memset(&servAddr, 0,sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port=htons(atoi(argv[2]));
	connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));

	while(1)
	{
		fputs("Insert message(q to quit):", stdout);
		fgets(message, sizeof(message), stdin);
		if(!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
		{
			break;
		}
		send(sock, message, strlen(message), 0);
		str_len = recv(sock, message, sizeof(message)-1, 0);
		message[str_len]=0;
		printf("Message from server : %s", message);
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}
