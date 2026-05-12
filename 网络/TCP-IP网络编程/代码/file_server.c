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
	SOCKET hServSock, hClientSock;
	FILE* fp;
	char buf[BUFF_SIZE];
	int read_cnt;

	SOCKADDR_IN servAddr, clientAddr;
	int clientAddr_sz;

	if(argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(-1);
	}
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		ErrorHandle("WSAStartup() failed");
	}
	fp = fopen("server.txt", "rb");
	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if((bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)))!=0)
	{
		ErrorHandle("bind() failed");
	}
	listen(hServSock, 5);
	clientAddr_sz = sizeof(clientAddr);
	hClientSock = accept(hServSock, (SOCKADDR*)&clientAddr,&clientAddr_sz);
	while(1)
	{
		read_cnt = fread((void*)buf, 1, BUFF_SIZE, fp);
		send(hClientSock, (char*)&buf, read_cnt, 0);
		if(read_cnt < BUFF_SIZE)
		{
			break;
		}
	}
	shutdown(hClientSock, SD_SEND);
	recv(hClientSock, (char*)buf, BUFF_SIZE, 0);
	printf("Message from client: %s \n", buf);
	fclose(fp);
	closesocket(hServSock);
	closesocket(hClientSock);
	WSACleanup();
	return 0;

}
