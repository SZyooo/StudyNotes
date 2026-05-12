#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	struct hostent *host;
	SOCKADDR_IN addr;
	if(argc != 2)
	{
		printf("Usage : %s <IP> \n", argv[0]);
		exit(-1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup() failed\n");
		exit(-1);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr=inet_addr(argv[1]);
	host=gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET);
	if(!host)
	{
		printf("gethostbyaddr failed");
		exit(-1);
	}
	printf("Official name : %s \n", host->h_name);
	int i = 0;
	for(;host->h_aliases[i]; ++i)
	{
		printf("Aliases %d : %s \n", i + 1, host->h_aliases[i]);
	}
	printf("Address type = %s", (host->h_addrtype == AF_INET) ? "IPv4" : "IPv6");
	for(i=0;host->h_addr_list[i];++i)
	{
		printf("IP address %d : %s\n", i+1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
	}
	WSACleanup();
	return 0;
}
