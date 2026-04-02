#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(-1);
}

void ShowSocketBufSize(SOCKET sock)
{
	int sndBuf, rcvBuf, state, len;
	len=sizeof(sndBuf);
	state=getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sndBuf, &len);
	if(state == SOCKET_ERROR)
	{
		ErrorHandling("getsockopt error");
	}
	len=sizeof(rcvBuf);
	state=getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvBuf, &len);
	if(state == SOCKET_ERROR)
	{
		ErrorHandling("getsockopt error");
	}
	printf("Input Buffer Size=%d", rcvBuf);
	printf("Output Buffer Size=%d", sndBuf);
	return;
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSocket;
	int send_buf, rcv_buf, state;
	if(WSAStartup(MAKEWORD(2, 2),&wsaData) !=0)
	{
		ErrorHandling("WSAStartup() failed");
	}
	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	ShowSocketBufSize(hSocket);
	send_buf=1024 * 3, rcv_buf=1024 * 3;
	state=setsockopt(hSocket, SOL_SOCKET, SO_SNDBUF, &send_buf, sizeof(send_buf));
	if(state==SOCKET_ERROR)
	{
		ErrorHandling("setsockopt failed");
	}
	state=setsockopt(hSocket, SOL_SOCKET, SO_RCVBUF,&rcv_buf, sizeof(rcv_buf));
	if(state==SOCKET_ERROR)
	{
		ErrorHandling("setsockopt failed");
	}
	ShowSocketBufSize(hSocket);
	WSACleanup();
	return 0;
}
