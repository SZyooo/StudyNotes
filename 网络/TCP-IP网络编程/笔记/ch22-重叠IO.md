# 重叠IO模型

重叠IO是windows下实现异步IO的具体机制。

这里的 **“重叠”** 是指：IO本身是异步的，调用IO之后立即返回去处理其他事情。处理其他事情和IO是“重叠”的。

## 一、重叠IO

要使用重叠IO，需要使用异步版本的IO函数和对象。

### 1.1 重叠IO套接字

```c
#include <winsock2.h>

SOCKET                                      //成功返回套接字句柄
WSASocket(
    int                 af,                 //地址族
    int                 type,               //套接字类型
    int                 protocol,           //两个套接字使用的协议信息
    LPWSAPROTOCOL_INFO  lpProtocolInfo,     //包含创建套接字的额外信息，不需要时传入NULL
    GROUP               g,                  //为扩展函数而预约的参数，可以使用0
    DWORD               dwFlags             //套接字属性信息
);
```

函数的前面三个参数和`socket`函数类似。其他参数可以给0或者`NULL`。重点是最后的`dwFlags`参数，我们需要传入`WSA_FLAG_OVERLAPPED`来创建重叠IO套接字：

```c
WSASocket(PF_INET, SOCK_STRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
```

### 1.2 执行重叠IO的`WSASend`函数

```c
#include <winsock2.h>

int                                                             //成功返回0，失败返回SOCKET_ERROR
WSASend(
    SOCKET                              s,                      //套接字句柄；如果传递的是具有重叠IO属性的套接字句柄，以重叠IO输出
    LPWSABUF                            lpBuffers,              //待传输的数据缓冲数组
    DWORD                               dwBufferCount,          //数据缓冲数组长度
    LPDWORD                             lpNumberOfBytesSent,    //保存实际发送的字节数
    DWORD                               dwFlags,                //改变数据传输属性。如传递`MSG_OOB`时发送OOB数据
    LPWSAOVERLAPPED                     lpOverlapped,           //WSAOVERLAPPED结构体变量地址，使用事件对象来查询IO是否结束
    LPWSAOVERLAPPED_COMPLETION_ROUTINE  lpCompletionRoutine     //传入Completion Routine函数的入口地址。用于IO结束时候的回调
);
```

数据缓冲的结构体定义为：

```c
typedef struct __WSABUF
{
    u_long      len; //数据大小
    char FAR*   buf; //缓冲地址
} WSABUF, *LPWSABUF;
```

保存事件对象的`WSAOVERLAPPED`结构体定义如下：

```c
typedef struct _WSAOVERLAPPED
{
    DWORD       Internal,
    DWORD       InternalHigh,
    DWORD       Offset,
    DWORD       OffsetHigh,
    WSAEVENT    hEvent
} WSAOVERLAPPED, *LPWSAOVERLAPPED;
```

其中的前面两个成员时操作系统内部使用的，我们初始化为0即可；`Offset`和`OffsetHigh`也是具有特殊用途的成员。我们仅仅关注最后成员`hEvent`。

另外，<font color=#0000FF>为了进行重叠IO，`WSASend`函数的`lpOverlapped`参数应该传递有效的结构体变量，而不是`NULL`，即使我们不使用事件对象查询IO状态</font>。否则，`WSASend`函数的第一个参数句柄所指的套接字将以阻塞模式工作。同时，<font color=#0000FF> 如果套接字不同，则传入的`lpOverlapped`地址也必须是不同的</font>，原因很简单：操作系统会在使用传入的`WSAOVERLAPPED`对象。

### 1.3 关于获取实际发送的字节数

`WSASend`函数的`lpNumberOfBytesSent`参数用来获取实际传输的数据大小。但是问题是：既然`WSASend`函数是异步的，我们如何立即知道传输了多少字节？

这里的处理方法按照情况分为两类：

**情况1：如果数据量不大**

这个时候函数可以立即完成数据传输，然后`WSASend`函数返回0，`lpNumberOfBytesSent`返回的是实际传输的数据大小，有意义

**情况2：数据量比较大**

`WSASend`函数无法立即完成数据传输，会返回`SOCKET_ERROR`，同时将`WSA_IO_PENDING`设置为错误代码，通过`WSAGetLastError`函数来获取。这个时候`lpNumberOfBytesSent`得到的就是无效数据，无意义

这种情况下我们应该使用下面这个函数来获取实际发送的数据量：

<span id="WSAGetOverlappedResult"></span>
```c
#include <winsock2.h>

BOOL                                        /*成功返回TRUE，失败返回FALSE*/
WSAGetOverlappedResult(
    SOCKET              s,                  /*进行重叠IO的套接字句柄*/
    LPWSAOVERLAPPED     lpOverlapped,       /*进行冲抵IO时传递的WSAOVERLAPPED结构体变量的地址值*/
    LPDWORD             lpcbTransfer,       /*保存实际传输的字节数的变量地址*/
    BOOL                fWait,              /*如果函数调用的时候仍然在IO的话，则如果这个参数为TRUE，则函数会等待IO完成；否则返回FALSE并且结束函数*/
    LPDWORD             lpdwFlags           /*调用WSARecv函数时，获取附加信息（例如OOB消息）；不需要时传入NULL*/
);
```

### 1.4 进行IO的`WSARecv`函数

```c
#include <winsock2.h>

int WSARecv(
    SOCKET                              s,
    LPWSABUF                            lpBuffers,
    DWORD                               dwBufferCount,
    LPDWORD                             lpNumberOfBytesRecvd,
    LPDWORD                             lpFlags,
    LPWSAOVERLAPPED                     lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE  lpCompletionRoutine
);
```

函数和`WSASend`是对称的

### 1.5 补充：关于Gather/Scatter IO

自己观察`WSASend`和`WSARecv`两个函数，会发现它们的发送和接收操作的都是一组缓冲数据。这个和Unix下的`writev`和`readv`有点类似。Windows下没有`writev`和`readv`的定义，但是我们可以利用`WSASend`和`WSARecv`来实现类似功能。

## 二、重叠IO的完成确认

重叠IO主要有以下两种方法来查询IO结果：

1. 利用`WSASend`、`WSARecv`的第6个参数`lpOverlapped`中的事件成员，基于事件对象
2. 利用`WSASend`、`WSARecv`的第7个参数，基于Completion Routine

### 2.1 使用事件对象

基于第6个参数使用事件对象的话，

- 完成IO的时候，`WSAOVERLAPPED`结构体变量引用的事件对象将变为signaled状态
- 为了验证IO的完成和完成结果，需要调用[`WSAGetOverlappedResult`](#WSAGetOverlappedResult)函数

### 2.2 使用Completion Routine

`WSASend`和`WSARecv`的最后一个参数是Completion Routine，作用是注册一个回调函数，即“Pending的IO完成时调用此函数”。

但是这个函数是操作系统调用的，如果任由操作系统调用的话，可能会破坏用户程序的正常执行。因此，操作系统预先定义了这样的规则：

**只有请求IO的线程处于alertable wait状态才能调用Completion Routine函数**

我们可以通过下面四个函数让我们的线程进入alertable wait状态：

- `WaitForSingleObjectEX` : 在`WaitForSingleObject`函数的基础上添加了一个额外`BOOL`参数指定是否进入alertable wait状态
- `WaitForMultipleObjectEx` : 在`WaitForMultipleObject`函数的基础上添加了一个额外`BOOL`参数指定是否进入alertable wait状态
- `WSAWaitForMultipleEvents` : 在`WaitForMultipleEvents`函数的基础上添加了一个额外`BOOL`参数指定是否进入alertable wait状态
- `SleepEx` : 在`Sleep`函数的基础上添加了一个额外参数`BOOL`指定是否进入alertable wait状态

下面是Completion Routine的签名：

```c
void CALLBACK CompletionROUTINE(
    DWORD           dwError,            //错误信息，没有错误写入0
    DWORD           cbTransferred,      //实际收发的字节数
    LPWSAOVERLAPPED lpOverlapped,       //传入WSASend或者WSARecv的lpOverlapped参数
    DWORD           dwFlags             //传入调用IO传入的特性信息或者0
);
```

前面说过，因为操作系统需要使用参数`lpOverlapped`，因此即使我们使用Completion Routine，我们也要传递一个有效的`WSAOVERLAPPED`对象。**我们可以利用这个对象中的`hEvent`指针来传递我们自己的额外参数**。因为这个指针是留给开发者使用的。我们在Completion Routine里面可以读取这个`hEvent`指针。
