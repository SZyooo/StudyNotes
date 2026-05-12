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
	SOCKET servSock;
	char message[BUFF_SZ];
	int strlen;
	int client_addr_sz;
	SOCKADDR_IN servAddr, clientAddr;
	if(argc != 2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(-1);
	}

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() failed");
	}

	servSock=socket(PF_INET, SOCK_DGRAM, 0);
	if(servSock==INVALID_SOCKET)
	{
		ErrorHandling("socket() failed");
	}
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if(bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
	{
		ErrorHandling("bind() failed");
	}

	while(1)
	{
		client_addr_sz = sizeof(clientAddr);
		strlen = recvfrom(servSock, message, BUFF_SZ, 0, (SOCKADDR*)&clientAddr, &client_addr_sz);
		sendto(servSock, message, strlen, 0, (SOCKADDR*) &clientAddr, sizeof(clientAddr));
	}
	closesocket(servSock);
	WSACleanup();
	return 0;
}
