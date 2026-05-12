#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

void ErrorHandling(char* message);

int main(int argc, char* argv[]){
	WSADATA wsaData;
	SOCKET hScoket;
	SOCKADDR_IN servAddr;

	char message[30] = { 0 };
	int strlen;
	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
	}
	hScoket = socket(PF_INET, SOCK_STREAM, 0);
	if(hScoket == INVALID_SOCKET) {
		ErrorHandling("socket() error!");
	}
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	if(connect(hScoket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		ErrorHandling("connect() error!");
	}
	strlen = recv(hScoket, message, sizeof(message) - 1, 0);
	if(strlen == -1) {
		ErrorHandling("recv() error!");
	}
	printf("Message from server: %s\n", message);
	closesocket(hScoket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}