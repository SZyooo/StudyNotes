# 实现基于UDP的服务器端/客户端

## 一、UPD

### 1.1 UDP套接字的特点：

- 提供的是不可靠的数据传输
- 没有ACK和SEQ
- 报文结构简单，性能比TCP更好

> **[NOTE]** TCP和UDP之间的区别主要就是是否有流控制。TCP提供流控制，而UDP不提供。

### 1.2 UDP的功能

IP层负责将消息发送到网络上目标主机，而UDP则负责将之转发给指定端口号的UDP套接字。

## 二、实现基于UDP的服务器端/客户端

### 2.1 无连接状态

UDP套接字之间没有所谓的连接过程，也就没有任何的连接状态。UDP主要需要完成下面两个事情：

1. 创建套接字
2. 发送/接受数据

### 2.2 只需要一个UDP套接字

和**TCP套接字是一对一**不同的是，UDP套接字不存在一对一的连接关系，因此无论是服务器还是客户端，都只需要一个UDP套接字即可。

### 2.3 基于UDP的数据I/O函数

因为UDP套接字之间没有连接，所以每次发送都需要指定目标的IP和端口。

#### 2.3.1 发送

```c
#include <sys/socket.h>

ssize_t                     //成功返回传输的字节数，否则返回-1
sendto(
    int sock,               //UDP套接字描述符
    void* buf,              //发送数据缓冲
    size_t nbytes,          //待传输的数据长度 
    int flags,              //可选参数，没有则填入0
    struct sockaddr* to,    //目标地址
    socklen_t addrlen       //`to`参数的长度
);
```

#### 2.3.2 接收

```c
#include <sys/socket.h>
ssize_t                     //成功返回接受的字节数，否则返回-1
recvfrom(
    int sock,               //接收数据的UDP套接字文件描述符
    void* buff,             //保存接受数据的缓冲地址
    size_t nbytes,          //可接受的最大字节数
    int flags,              //可选参数，没有传入0
    struct sockaddr* from,  //发送端地址
    socklen_t* addrlen      //`from`参数的长度
)
//函数默认是阻塞的
```

> **[NOTE]** 上面的`recvfrom`函数的`nbytes`参数决定了UDP的数据传输是有大小限制的。使用UDP的情况下，发送方发送了一个UDP报文之后，接收方必须在一次接收过程中接收全部，否则的话数据就会丢失。


### 2.4 UDP套接字的自动分配

TCP套接字在调用`connect`函数的时候会自定绑定IP和端口号；类似的，函数`sendto`会自动给UDP套接字分配IP和端口号。

> 我们可以使用`bind`函数手动给套接字分配IP和端口号。`sendto`函数会在套接字没有IP和端口号的时候自动为它绑定。

## 三、UDP数据传输特性和`connect`函数

### 3.1 存在数据边界

TCP数据传输使用了缓存机制，因此是不具有数据边界的，即数据传输过程中调用I/O次数不具有任何意义。

和TCP不同的是，UDP的每次发送都是独立的，这意味着必须在一次发送-接收之间把数据交换完毕。如果发送方发送了3次数据，则接收方必须接收3次才能完整接收数据。

### 3.2 已连接的UDP套接字和未连接的UDP套接字

使用TCP套接字的时候，需要往套接字内注册目标主机的IP和端口。这样在连接的期间内，TCP套接字总是知道往哪里发送数据。

使用UDP看上去好像不需要这样注册。然而实际上并不是。在调用`sendto`函数的时候，会执行下面三个步骤：

- 向UDP套接字注册目标的IP和端口号
- 传输数据
- 删除UDP套接字中注册的IP和端口

可以看出来仍然存在注册的过程。并且每次独立的`sendto`函数都会完整地执行这三步。

很明显，这样会影响通信的性能。不过，我们可以手动为UDP套接字注册目标IP和端口号，这样每次发送就可以省略步骤一、三。

我们在这里使用`connect`函数。`connect`函数将TCP端口连接到另一个TCP套接字，而对于UDP套接字，则是提前将目标主机的IP和端口号记录给它。我们称这种UDP套接字为**已连接**的UDP套接字，其他套接字为**未连接的套接字**。

对于连接了的套接字，我们无需再使用`sendto`和`recvfrom`函数，可以直接使用`write`或者`read`（在linux上，windows上是`send`和`recv`)

## 四、Windows实现

windows和linux一样提供接口和功能完全一致的接口：

```c
#include <winsock2.h>

int 
sendto(
    SOCKET s, 
    const char* buf, 
    int len, 
    int flags, 
    const struct sockaddr* to, 
    int tolen
);
```

```c
#include <winsock2.h>

int
recvfrom(
    SOCKET s,
    char* buf,
    int len,
    int flag,
    struct sockaddr* from,
    int* from_len
);

```
