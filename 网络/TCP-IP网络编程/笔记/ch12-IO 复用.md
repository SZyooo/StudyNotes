# I/O 复用

## 一、基于I/O复用的服务器端

简单理解，复用就是在单个通信频道传递多个信号的技术。经典的有两种复用技术：

- 时分复用
- 频分复用

I/O复用可以减少服务多个客户端的时候需要的进程数量。

## 二、`select`函数

### 2.1 `select`函数的功能

一般情况下，我们一个进程只能监听一个套接字。使用`select`函数之后，我们可以同时监听一组套接字，在其中任何一个套接字发生变化的时候，我们都可以进行响应。

`select`函数支持对套接字组监听下面三类事件：

- **可读事件**：这个事件表示某个套接字发生了可读事件，例如新的数据到达，连接已接受、对端关闭连接等
- **可写事件**：这个事件表示某个套接字支持外部写入，例如缓冲区有新的写入空间
- **异常事件**：这个事件表示某个套接字发生了异常

同时，我们可以给`select`设置一个监听的超时。如果没有任何套接字发生变化的话，则超时后`select`函数返回。

### 2.2 设置文件描述符组

我们需要将需要监听的文件描述符按照三类事件组织在一起传递给`select`。将文件描述符组织在一起的数据结构是`fd_set`。

`fd_set`是位集合，每一个文件描述符对应一个位，每个位的值为0或者1；文件描述符本身是一个整型值，它的值表示在这个位集合中的下标。

对于这个bit-set，操作系统提供了下面四个宏对它进行操作：

- **`FD_ZERO(fd_set* )`**: 将`fd_set`所有位置为0
- **`FD_SET(int, fd_set* )`**: 将指定位设置为1
- **`FD_CLR(int, fd_set* )`**: 将指定位设置为0
- **`FD_ISSET(int, fd_set* )`**: 查询指定位是否被设置了

### 2.3 函数签名

```c
#include <sys/select>
#include <sys/time.h>


struct timeval{
    long tv_sec;    //秒
    long tv_usec;   //微秒
};

int                                     //失败返回-1；有事件发生返回该描述符值；超时返回0
select(
    int                     maxfd,      //监视的描述符中最大值+1
    fd_set*                 readset,    //监听可读事件的描述符组
    fd_set*                 writeset,   //监听可写事件的描述符组
    fd_set*                 excepset,   //监听异常事件的描述符组
    const struct timeval*   timeout     //select函数的监听超时
);
```

- 如果没有任何描述符触发事件，则`select`一直阻塞直到超时。
- 如果有任何集合的任何描述符发生变换，则`select`返回

`select`函数会在调用结束后清理所有位，只保留那些触发事件的位。


### 2.4 查询

我们调用`select`之后，需要遍历查询`fd_set`，一般需要遍历0-`maxfd`的所有值，逐一调用`FD_ISSET`检查。

> **[NOTE]** Unix下，文件描述符的分配采取的策略是分配最小的空闲值。初始状态下，每个进程会默认占用三个文件描述符：0-`stdin`，1-`stdout`，2-`stderr`。因此用户创建的文件描述符值会从3开始递增。

## 三、Windows实现

### 3.1 Windows下的`select`函数

```c
#include <winsock2.h>

int
select(
    int                     nfds,
    fd_set*                 readfds,
    fd_set*                 writefds,
    fd_set*                 exceptfds,
    const struct timeval*   timeout
);
```
> 第一个参数是为了和Unix兼容，在windows下没有实际意义。

其中，`fd_set`的实现和Unix下稍微有一点不同：

```c
typedef struct fd_set
{
    u_int   fd_count;
    SOCKET  fd_array[FD_SETSIZE];
} fd_set;
```

由于Windows下的套接字描述符的分配值是随机的，不像Linux下所有文件描述符的值是从0开始递增的，所以无法类似Linux那样采用位数组。

不过为了兼容性，Windows同样支持`FD_ZERO`等4个宏。
