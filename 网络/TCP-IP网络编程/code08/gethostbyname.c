#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	struct hostent* host;
	if(argc!=2)
	{
		printf("USage : %s <addr> \n", argv[0]);
		exit(-1);
	}
	if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0)
	{
		printf("WSAStartup() failed");
		exit(-1);
	}
	host=gethostbyname(argv[1]);
	if(!host)
	{
		printf("gethostbyname failed");
		exit(-1);
	}
	int i = 0;
	for(;host->h_aliases[i]; ++i)
	{
		printf("Aliases[%d] : %s", i+1, host->h_aliases[i]);
	}
	printf("Address type = %s", (host->h_addrtype==AF_INET)?"IPv4" : "IPv6");
	for(i = 0; host->h_addr_list[i]; ++i)
	{
		printf("Address[%d] : %s", i + 1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
	}
	WSACleanup();
	return 0;
}
