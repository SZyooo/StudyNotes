#include <stdio.h>
#include <windows.h>
#include <process.h>

#define NUM_THREAD 50

long long num=0;
CRITICAL_SECTION cs;

unsigned WINAPI ThreadInc(void* arg)
{
	int i;
	EnterCriticalSection(&cs);
	for(i=0;i<50000000;++i)
		num+=1;
	LeaveCriticalSection(&cs);
	return 0;
}

unsigned WINAPI ThreadDec(void* arg)
{
	int i;
	EnterCriticalSection(&cs);
	for(i=0;i<50000000;++i)
		num-=1;
	LeaveCriticalSection(&cs);
	return 0;
}

int main(int argc, char* argv[])
{
	HANDLE tThreads[NUM_THREAD];
	int i;
	InitializeCriticalSection(&cs);
	for(i=0;i<NUM_THREAD;i++)
	{
		if(i % 2)
			tThreads[i]=(HANDLE)_beginthreadex(NULL, 0, ThreadInc, NULL, 0, NULL);
		else
			tThreads[i]=(HANDLE)_beginthreadex(NULL, 0, ThreadDec, NULL, 0, NULL);	
	}
	WaitForMultipleObjects(NUM_THREAD, tThreads, TRUE, INFINITE);
	DeleteCriticalSection(&cs);
	printf("result : %lld \n", num);
	return 0;
}
