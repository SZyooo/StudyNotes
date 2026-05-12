#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUFF_SIZE 1024

void ErrorHandle(const char* err)
{
	fputs(err, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hListenSock, hRecvSock;
	SOCKADDR_IN listenAddr, recvAddr;

	int recvAddrSz;

	WSABUF dataBuf;
	WSAEVENT evtObj;
	WSAOVERLAPPED overlapped;

	char buf[BUFF_SIZE];
	int recvBytes=0, flags=0;
	if(argc !=2 )
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandle("WSAStartup error") ;
	}

	hListenSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family=AF_INET;
	listenAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	listenAddr.sin_port=htons(atoi(argv[1]));
	if(bind(hListenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr))==SOCKET_ERROR)
	{
		ErrorHandle("bind() error");
	}
	if(listen(hListenSock, 5)==SOCKET_ERROR)
	{
		ErrorHandle("listen() error");
	}

	recvAddrSz=sizeof(recvAddr);
	hRecvSock=accept(hListenSock, (SOCKADDR*)&recvAddr, &recvAddrSz);

	evtObj=WSACreateEvent();
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent=evtObj;
	dataBuf.len = BUFF_SIZE;
	dataBuf.buf = buf;
	if(WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, NULL) == SOCKET_ERROR)
	{
		if(WSAGetLastError() == WSA_IO_PENDING)
		{
			puts("Background data receive");
			WSAWaitForMultipleEvents(1, evtObj, TRUE, WSA_INFINITE, FALSE);
			WSAGetOverlappedResult(hRecvSock, &overlapped, &recvBytes, FALSE, NULL);
		}
		else
		{
			ErrorHandle("WSARecv error");
		}
	}
	printf("Received message: %s\n", buf);
	WSACloseEvent(evtObj);
	closesocket(hRecvSock);
	closesocket(hListenSock);
	WSACleanup();
	return 0;
}
