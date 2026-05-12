#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 30


void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hListenSock, hClientSock;

	SOCKADDR_IN listen_sock_addr, client_sock_addr;
	int client_sock_addr_sz, strLen;
	char buf[BUF_SIZE];
	int result;

	fd_set read, except, readCopy, exceptCopy;

	struct timeval timeout;

	if(argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if(WSAStartup(MAKEWORD(2, 2),&wsaData)==SOCKET_ERROR)
	{
		ErrorHandling("WSAStartup() failed");
	}

	hListenSock=socket(AF_INET, SOCK_STREAM, 0);
	memset(&client_sock_addr, 0, sizeof(client_sock_addr));
	listen_sock_addr.sin_family=AF_INET;
	listen_sock_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	listen_sock_addr.sin_port=htons(atoi(argv[1]));

	if(bind(hListenSock, (SOCKADDR*)&listen_sock_addr, sizeof(listen_sock_addr))==SOCKET_ERROR)
	{
		ErrorHandling("bind() failed");
	}

	if(listen(hListenSock, 5)==SOCKET_ERROR)
	{
		ErrorHandling("listen() failed");
	}

	client_sock_addr_sz=sizeof(client_sock_addr);
	hClientSock=accept(hListenSock, (SOCKADDR*)&client_sock_addr, &client_sock_addr_sz);
	FD_ZERO(&read);
	FD_ZERO(&except);
	FD_SET(hClientSock, &read);
	FD_SET(hClientSock, &except);

	while(1)
	{
		readCopy=read;
		exceptCopy=except;

		timeout.tv_sec=5;
		timeout.tv_usec=0;
		result=select(0, &readCopy, 0, &exceptCopy, &timeout);
		if(result>0)
		{
			if(FD_ISSET(hClientSock, &exceptCopy))
			{
				strLen=recv(hClientSock, buf, BUF_SIZE-1, MSG_OOB);
				buf[strLen]=0;
				printf("Urgent message: %s\n", buf);
			}
			if(FD_ISSET(hClientSock, &readCopy))
			{
				strLen=recv(hClientSock, buf, BUF_SIZE-1, 0);
				if(strLen==0)
				{
					closesocket(hClientSock);
					break;
				}
				else
				{
					buf[strLen]=0;
					puts(buf);
				}
			}
		}
	}
	closesocket(hListenSock);
	WSACleanup();
	return 0;
}
