#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFF_SIZE 30


void ErrorHandle(const char* err)
{
	fputs(err, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSocket;
	FILE* fp;
	char buf[BUFF_SIZE];
	int read_cnt;
	SOCKADDR_IN servAddr;
	if(argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(-1);
	}
	if(WSAStartup(MAKEWORD(2, 2),&wsaData) != 0)
	{
		ErrorHandle("WSAStartup() failed");
	}
	fp = fopen("receive.txt", "wb");
	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));
	printf("Connecting to server %s:%s\n", argv[1], argv[2]);

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		int err_code = WSAGetLastError();
		printf("connect() error. Error Code : %d", err_code);
		exit(-1);
	}

	while ((read_cnt = recv(hSocket, buf, BUFF_SIZE, 0)) > 0)
	{
		fwrite((void*)buf, 1, BUFF_SIZE, fp);
		fputs(buf, stderr);
	}
		
	puts("Received file data");
	send(hSocket, "Thank you", 10, 0);
	printf("Sent thank you message\n");
	fclose(fp);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}
