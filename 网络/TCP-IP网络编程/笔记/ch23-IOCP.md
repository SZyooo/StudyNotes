# IOCP

**IOCP(Input Output Completion Port)** 是Windows上性能最好的I/O模型。

## 一、通过重叠IO理解IOCP

### 1.1 非阻塞模式的套接字

我们通过`WSASocket`加上`WSA_FLAG_OVERLAPPED`可以创建支持重叠IO的套接字。这些套接字可以被使用在重叠IO函数上。

但是有一点需要明确的是：**<font color="red">这里的异步IO或者说重叠IO，和套接字本身的阻塞行为是两个概念</font>**。

为什么这么说？因为我们在非重叠IO函数上使用这些套接字的话，结果仍然是阻塞的，例如普通的`read`、`write`以及`accept`等。

我们可以使用下面的接口来改变套接字的模式为非阻塞:

```c
SOCKET hListenSock;
int mode=1;
hListenSock=WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
ioctlsocket(hListenSock, FIONBIO, &mode); //mode=1的话，则改为非阻塞模式
```
这样做的目的之一是为了让`accept`函数不会被阻塞：

- 如果在没有客户端连接请求的情况下调用`accept`函数，将直接返回`INVALID_SOCKET`，并且调用`WSAGetLastError`会返回`WSAEWOULDBLOCK`。
- 通过`accept`返回的套接字默认也是非阻塞的

### 1.2 一个经典的单线程重叠IO服务器：

下面的代码给出了一个经典的单线程重叠IO服务器的实现框架：

```c
int main(int argc, char* argv[])
{
    //...初始化网络接口

    //构造支持重叠IO的非阻塞监听套接字
    SOCKET hListenSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    int mode=1;
    ioctlsocket(hListenSock, FIONBIO, &mode);

    //...

  
    while(1)
    {
        SleepEx(100, TRUE);//短暂进入alertable wait state
        hClientSock=accept(hListenSock, (SOCKADDR*)&recvAddr, &recvAddrSz);//无阻塞地尝试接收连接
        if(hClientSock==INVALID_SOCKET)
        {
            if(WSAGetLastError() == WSAEWOULDBLOCK)
                continue;//没有连接，继续
            else
                //处理错误
        }
        //连接成功
        LPWSAOVERLAPPED lpOverlapped = malloc(WSAPVERLAPPED);
        WSABUF buf;
        //... 正确填充buf
        int recvedBytes, flag;
        WSARecv(hClientSock, &buf, 1, &recvedBytes, &flag, lpOverlapped, ReadRoutine);
    }
    //...
}

void CALLBACK ReadRoutine(DWORD err, DWORD szRecv, ...)
{
    if(szRecv == 0)
    {
        //...对方关闭连接
    } 
    else
    { 
        //处理消息
    }
}
```

我们可以看到：在服务器的主循环中，我们必须抽出一段时间进入到alertable wait状态，以让我们的IO结束ROUTINE被操作系统调用。这种不停地进入等待状态很显然会影响服务器的性能。因此，我们很自然能想到使用额外的IO线程。

IOCP采用的服务器端模型就是基于重叠IO加上多线程设计的：IOCP会创建专用的IO线程，该线程会跟所有客户端进行IO。

## 二、IOCP

**完成端口（Input Ouput Completion Port）**是一个内核对象。它是由内核维护的一个队列，每一个关联到它的重叠IO请求完成之后，内核就会添加一个包含:1、完成状态;2、传输字节数;3、`OVERLAPPED`结构体等的信息到队列中。

我们创建合适数量的线程（一般建议是内核数量x1~2），在线程里面专门等待IOCP队列出现完成的IO事件并且进行处理。

因此，使用IOCP，我们需要明确下面几点：

- **创建IOCP对象**
- **将（必须支持重叠IO）套接字关联到IOCP对象**
- **创建合适数量的线程，并且在线程内部等待IOCP队列有新的数据**

### 2.1 创建“完成端口”

```c
#include <windows.h>

HANDLE                                              //成功返回IOCP句柄，失败返回NULL
CreateIoCompletionPort(
    HANDLE              FileHandle,                 //创建IOCP对象必须传递INVALID_HANDLE_VALUE
    HANDLE              ExistingCompletionPort,     //创建IOCP对象传递NULL
    ULONG_PTR           CompletionKey,              //创建IOCP对象传递0
    DWORD               NumberOfConcurrentThreads   //在IOCP就绪的时候被唤醒的线程最大数量；填入0表示采用系统的CPU个数
);
```

> 创建完成端口只有最后一个参数有意义。

### 2.2 关联端口对象和套接字

关联和创建完成端口使用的是同样的接口：
```c
#include <windows.h>

HANDLE                                              //成功返回IOCP句柄，失败返回NULL
CreateIoCompletionPort(
    HANDLE              FileHandle,                 //要连接到IOCP的套接字句柄
    HANDLE              ExistingCompletionPort,     //进行关联的IOCP句柄
    ULONG_PTR           CompletionKey,              //这个值将在后面传递给被IOCP唤醒的线程
    DWORD               NumberOfConcurrentThreads   //只要第二个参数非NULL这个参数就会被忽略
);
```

### 2.3 等待IOCP队列就绪

```c
#include <windows.h>

BOOL                                            //成功返回TRUE，失败返回FALSE
GetQueuedCompletionStatus(
    HANDLE                  CompletionPort,     //等待的IOCP句柄
    LPDWRD                  lpNumberBytes,      //接收IO传输的数据大小变量的地址
    PULONG_PTR              lpCompletionKey,    //接收使用CreateIoCompletionPort关联套接字和IOCP对象时候传递的CompletionKey值的变量地址
    LPOVERLAPPED            *lpOverlapped,      //调用`WSASend`、`WSARecv`等函数传递的`OVERLAPPED`结构体地址的变量地址值
    DWORD                   dwMilliseconds      //超时信息。超时之后返回FALSE；传递`INFINITE`程序将阻塞知道有新的完成IO进入IOCP
);    
```
> `OVERLAPPED`和`WSAOVERLAPPED`其实是同一个东西。前者表示通用IO，后者表示用于套接字IO（Windows Socket API）

> **[<font color="red">注意</font>]** 在我们使用普通的重叠IO的时候，如果没有使用事件对象的话，我们可以利用`OVERLAPPED::hEvent`成员传递我们自定义的数据。但是如果使用IOCP的话，我们<font color="red"> 必须</font>将其初始化为0。
> 我们可以使用这样的技巧来取代之前的做法：我们将`OVERLAPPED`对象作为我们自定义结构体的第一个成员，这样的话我们可以将结构体地址作为`OVERLAPPED`对象传递给`WSARect`等重叠函数。