#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClientSock;
	SOCKADDR_IN servAddr, clientAddr;
	TIMEVAL timeout;
	fd_set reads, cpyReads;
	int addrsz;
	int strLen, fdNum, i;
	char buf[BUF_SIZE];

	if(argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() failed");
	}
	hServSock=socket(AF_INET, SOCK_STREAM, 0);
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAddr.sin_port=htons(atoi(argv[1]));
	if(bind(hServSock, (struct sockaddr*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
	{
		ErrorHandling("bind() failed");
	}
	if(listen(hServSock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen() failed");
	}
	FD_ZERO(&reads);
	FD_SET(hServSock, &reads);
	while(1)
	{
		cpyReads = reads;
		timeout.tv_sec=5;
		timeout.tv_usec=0;
		if((fdNum=select(0, &cpyReads, 0, 0, &timeout))==-1)
		{
			break;
		}
		else if(fdNum==0) continue;
		for(i = 0; i < reads.fd_count; i++)
		{
			if(FD_ISSET(reads.fd_array[i], &cpyReads))
			{
				if(reads.fd_array[i]==hServSock)
				{
					addrsz=sizeof(clientAddr);
					hClientSock=accept(hServSock, (struct sockaddr*)&clientAddr, &addrsz);
					FD_SET(hClientSock, &reads);
					printf("connected client: %lld\n", hClientSock);
				}
				else
				{
					strLen=recv(reads.fd_array[i], buf, BUF_SIZE-1, 0);
					if(strLen==0)
					{
						closesocket(reads.fd_array[i]);
						FD_CLR(reads.fd_array[i], &reads);
						printf("closed client: %lld\n", cpyReads.fd_array[i]);
					}
					else
					{
						send(reads.fd_array[i], buf, strLen,0);
					}
				}
			}
		}
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}
