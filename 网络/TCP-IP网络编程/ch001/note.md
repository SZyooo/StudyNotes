## 一、套接字（Socket）

套接字是操作系统提供的网络数据传输用的软件设备。大体上分为两类：客户端套接字和服务端套接字。

这里以Linux的接口为例，介绍创建两类套接字的步骤。

### 1.1 客户端套接字

- 1.创建套接字

```
#include <sys/socket.h>
int socket(int domain, int type, int protocal);
```

- 2.分配地址信息

```
#include <sys/socket.h>
int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen);
```

- 3.监听

```
#include <sys/socket.h>
int listen(int sockfd, int backlog);
```

- 4.接受链接请求

```
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

### 1.2 服务器套接字

- 1.创建套接字

```
#include <sys/socket.h>
int socket(int domain, int type, int protocal);
```

- 2.送连接请求

```
#include <sys/socket.h>
int connet(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen);
```

## 二、Linux的文件

**在Linux中，socket操作与文件操作没有区别！**

在Linux系统下，socket也被认为是文件的一种，因此可以自然使用文件I/O的一系列操作。

在Windows系统下，文件和socket是两类不一样的对象。

### 2.1 文件描述符

**文件描述符是系统分配给文件或套接字的整数。**Linux系统将前3个文件描述符固定分配给标准流：

文件描述符 | 对象
|:-|:-|
|0|标准输入：Standard Input|
|1|标准输出：Standard Output|
|2|标准错误：Standard Error|

而对于普通文件包括socket，经过创建之后才会被分配文件描述符。

> **NOTE**
> 文件描述符有时候被称为文件句柄。但是“句柄”这个属于主要在Windows语境中使用

#### 2.1.1 打开文件

```C
int                     //成功：文件描述符；失败返回-1 
open(
    const char* path,   //文件名字
    int flag            //文件打开模式
);
```

<center>文件打开模式</center>

|打开模式|含义|
|:-|:-|
|O_CREAT|必要时创建文件|
|O_TRUNC|删除全部现有数据|
|O_APPEND|维持现有数据，追加数据|
|O_RDONLY|只读打开|
|O_WRONLY|只写打开|
|O_RDWR|读写打开|

#### 2.1.2 关闭文件

```C
int         //成功:0，失败：-1 
close(
    int fd  //文件描述符
);
```

#### 2.1.3 写入文件

```C
ssize_t                 //成功返回写入字节数，失败范围-1
write(
    int fd,             //文件描述符
    const void *buf,    //传输数据地址
    size_t nbytes       //传输字节数
);
```

> **[NOTE]**
> `size_t`即`unsigned int`；
> `ssize_t`则是`signed int`

#### 2.1.4 读取文件

```C
ssize_t             //成功返回读取的字节数（遇到结尾返回0）；失败返回-1
read(
    int fd,         //文件描述符
    void *buf,      //接受数据的缓冲区
    size_t nbytes   //接受数据的最大字节数
);
```

## 三、Windows平台的实现

### 3.1 头文件和库

- 导入头文件 `<winsock2.h>`
- 连接`ws2_32.lib`库

### 3.2 Winsock的初始化

和Linux不同的地方是，Windows下使用套接字之前需要先初始化：

```C
#include <winsock2>
int                         //成功返回0；失败返回非0的错误码
WSAStartup(
    WORD wVersionRequested, //设置Winsock版本
    LPWSADATA lpWSAData     //WSADATA结构体变量的地址
)
```

>`WORD`是`typedef`的`unsigned short`类型。对于版本号而言，`WORD`的高8位是副版面本号，低8位是主版本号。如果版本为1.2，则`WORD`的值为0x0201。可以使用`MAKEWORKD`宏（`MAKEWORD(1, 2)`）

### 3.3 注销Winsock

有初始化就有注销。使用下面的函数来注销Winsock库：

```
int             //成功返回0；失败返回SOCKET_ERROR
WSACleanup(void);
```

### 3.4 Windows的套接字相关函数

- **创建**

```
#include <winsock2.h>
SOCKET                  //成功返回套接字句柄；失败返回INVALID_SOCKET
socket( int af,         
        int type,
        int protocal
);
```

- **绑定**

```
#include <winsock2.h>
int                                         //成功返回0；失败返回SOCKET_ERROR
bind(   SOCKET s,
        const struct ssockaddr* name,
        int namelen
);
```

- **监听**

```
#include <winsock2.h>

int                         //成功返回0；失败返回SOCKET_ERROR
listen( SOCKET  s,
        int     backlog
);
```

- **接受**

```
#include <winsock2.h>

SOCKET
accept(
        SOCKET  s,
        struct sockaddr* addr,
        int* addrlen
);
```

- **连接**

```
#include <winsock2>

int
connet(
        SOCKET s,
        const struct sockaddr* name,
        int namelen
);
```

- **关闭**

```
#include <winsock2.h>

int
closesocket(
    SOCKET s
);
```

- **写入**

```
#include <winsock2.h>

int
send(   SOCKET s,
        const char* buf,
        int len,
        int flags
);
```

- 读取

```
#include <winsock2.h>

int
recv(   SOCKET s,
        const char* buf,
        int len,
        int flags
);
```
