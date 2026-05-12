#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUFF_SIZE 1024

void ErrorHandling(const char* err)
{
	fputs(err, stderr);
	fputc('\n', stderr);
	exit(-1);
}


