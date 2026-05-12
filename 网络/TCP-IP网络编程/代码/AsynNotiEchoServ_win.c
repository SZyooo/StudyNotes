#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUFF_SIZE 100

void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	int i=0;
	for(i=idx; i < total; ++i)
		hSockArr[i]=hSockArr[i+1];
}

void CompressEvents(WSAEVENT hEventArr[], int idx, int total)
{
	int i=0;
	for(i=idx; i < total; ++i)
		hEventArr[i]=hEventArr[i+1];
}
void ErrorHandling(const char* err)
{
	fputs(err, stderr);
	fputc('\n', stderr);
	exit(-1);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClientSock;
	SOCKADDR_IN servAddr, clientAddr;
	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT newEvent;
	WSANETWORKEVENTS netEvents;

	int numOfClientSock=0;
	int strLen, i;
	int posInfo, startIndex;
	int clientAddrLen;
	char msg[BUFF_SIZE];

	if(argc != 2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() failed");
	}
	hServSock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAddr.sin_port=htons(atoi(argv[1]));

	if(bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
	{ ErrorHandling("bind failed"); } if(listen(hServSock, 5)==SOCKET_ERROR)
	{
		ErrorHandling("listen failed");
	}

	newEvent=WSACreateEvent();
	if(WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
	{
		ErrorHandling("WSAEventSelect failed");
	}
	hSockArr[numOfClientSock]=hServSock;
	hEventArr[numOfClientSock]=newEvent;
	numOfClientSock++;
	while(1)
	{
		posInfo=WSAWaitForMultipleEvents(numOfClientSock, hEventArr, FALSE, WSA_INFINITE, FALSE);
		startIndex = posInfo - WSA_WAIT_EVENT_0;
		for(int i = startIndex; i < numOfClientSock; ++ i)
		{
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);
			//到尽头了或者等待超时
			if((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
				continue;
			else
			{
				sigEventIdx = i;
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
				if(netEvents.lNetworkEvents & FD_ACCEPT)
				{
					if(netEvents.iErrorCode[FD_ACCEPT_BIT]!=0)
					{
						puts("accept error");
						break;
					}
					clientAddrLen=sizeof(clientAddr);
					hClientSock=accept(hSockArr[sigEventIdx], (SOCKADDR*)&clientAddr, &clientAddrLen);
					newEvent=WSACreateEvent();
					WSAEventSelect(hClientSock, newEvent, FD_READ | FD_CLOSE);
					hEventArr[numOfClientSock]=newEvent;
					hSockArr[numOfClientSock]=hClientSock;
					numOfClientSock++;
					puts("connected new client...");
				}

				if(netEvents.lNetworkEvents & FD_READ)
				{
					if(netEvents.iErrorCode[FD_READ_BIT]!=0)
					{
						puts("read error");
						break;
					}
					strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
					send(hSockArr[sigEventIdx], msg, strLen, 0);
				}

				if(netEvents.lNetworkEvents & FD_CLOSE)
				{
					if(netEvents.iErrorCode[FD_CLOSE_BIT] != 0)
					{
						puts("close error");
						break;
					}
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);
					numOfClientSock --;
					CompressEvents(hEventArr, sigEventIdx, numOfClientSock);
					CompressSockets(hSockArr, sigEventIdx, numOfClientSock);
					puts("closed client...");
				}
			}
		}
	}
	WSACleanup();
	return 0;
}
