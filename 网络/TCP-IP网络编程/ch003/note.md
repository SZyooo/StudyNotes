# 地址族与数据序列

## 一、IP地址和端口号

- **IP(Internet Protocol)**：为了收发网络数据分配给计算机的值
- **端口号**：区分程序中创建的套接字二分配给套接字的序号

### 1.1 网络地址

电脑连接网络并且收发数据必须具备IP地址。IP地址分为两类：

- IPv4：4字节地址族
- IPv6：6字节地址族

IPv4标准的4字节IP地址分为**网络地址**和**主机地址**；整体上，有5类IPv4地址（A、B、C、D、E）：

|分类|2进制|首字节数值范围|用处|
|:-|:-|:-|:-|
|A|[0===][####][####][####]|0-127|网络地址|
|B|[10==][====][####][####]|128-191|网络地址|
|C|[110=][====][====][####]|192-223|网络地址|
|D|[1110][====][====][====]|224-239|多点广播|
|E|[1111][====][====][====]|240-255|为以后保留|

> "="表示网络地址；"#"表示主机地址

网络地址是为了区分网络而设置的；主机地址是在一个局域网内部区分主机而设置的。

### 1.2 网络地址分类

根据前面的表格可知，根据第一个字节就能得到一个IP地址所属类别。

### 1.3 区分套接字的端口号

IP地址只能定位主机，无法区分不同的套接字。区分不同套接字借助的是端口号。

端口号由2个字节16-bit构成，值的范围为0-65535。不过，**0-1023**是知名端口号，一般分配给特定应用程序，因此应当分配在这个范围之外的端口号值。

虽然TCP套接字端口号不能重复、UDP套接字端口号不能重复，但是**TCP和UDP之间的端口号可以重复**。

## 二、地址信息的表示

> 这里主要介绍IPv4地址

### 2.1 表示IPv4地址的结构体

```C
struct sockaddr_in
{
    sa_family_t         sin_family;         //地址族（Address Family）
    uint16_t            sin_port;           //16位TCP/UDP端口
    struct in_addr      sin_addr;           //32位IP地址
    char                sin_zero[8];        //填充用
};


struct in_addr
{
    in_addr_t           s_addr;             //32-bit IPv4地址
};
```

### 2.2 `sockaddr_in`成员分析

- **sin_family**：地址族。
    不同协议族采用的地址族不一样，IPv4采用的是4字节地址族，IPv6采用的是16字节地址族。
    |地址族（Address Family）|含义|
    |:-|:-|
    |AF_INET|IPv4网络协议中使用的地址族|
    |AF_INET6|IPv6网络协议中使用的地址族|
    |AF_LOCAL|本地通信采用的UNIX协议的地址族|

- **sin_port**：16位端口号（网络字节序）

- **sin_addr**：32bit的IP地址信息，网络字节序保存。

- **sin_zero**：为了使结构体`sockaddr_in`和结构体`sockaddr`保持一致而引入的填充字节，必须是0

### 2.3 `sockaddr`和`sockaddr_in`

我们使用bind函数的时候，一般是这样的：

```C
struct sockaddr_in serv_addr;
//...
if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
    //error
//...
```

这里有两个细节可以注意到：

- 第二个参数进行了强转
- 多给了第三个参数

对于第一点来说，需要介绍一下`sockaddr`和`sockaddr_in`之间的关系：

`sockaddr`的定义：

```C
struct sockaddr
{
    sa_family_t sin_family;     //地址族
    char        sa_data[14];    //地址信息
};
```

这个结构体是一个通用类型的定义，并非只为IPv4定义；而`sockaddr_in`则是专门为了保存IPv4地址信息的，可以理解为是`sockaddr`的一个“子类”。因此，虽然`sockaddr_in`是专门为了IPv4定义的结构体，仍然需要添加一个成员变量`sin_family`来指定地址族；同时需要添加`sin_zero`来填充结构体使之与`sockaddr`大小一致。


对于第二点来说，我们可以很容易看出来：`sa_data[14]`的大小是无法装进一个IPv6地址的。这是因为设计这个结构体的时候还没有设计IPv6。因此后面为IPv6专门设计了结构体`sockaddr_in6`。这个结构体显著大于`sockaddr`，但是`bind`这种函数仍然接受的是一个`sockaddr`的指针。因此采用的方法是强制类型转换加上第三个参数来指定实际传输的对象的大小。


## 三、网络字节序与地址变换

### 3.1 字节序

CPU向内存保存数据的方式有两种：

- 大端序（Big Endian）：高位字节放在低位地址
- 小端序（Little Endian）：高位字节存放在高位地址

我们成一个CPU采用的字节序为**主机字节序（Host Byte Order）**。

> 主流的Intel系列CPU采用的是小端序

因此很显然，我们不能直接在网络上随便发送字节流：如果接发两方发送的字节序不同，数据解析会错乱。

于是网路上发送的字节流有一个统一的标准：**全部采用大端序**。


### 3.2 字节序转换

Linux和Windows都提供了下面的函数来在主机字节序和网络字节序之间转换：

- `unsigned short htons(unsigned short);`
- `unsigned short ntohs(unsigned short);`
- `unsigned long htonl(unsigned long);`
- `unsigned long ntohl(unsigned long);`

其中，

- `htons`的`h`表示“host”，`n`表示“network”
- `htons`的`s`表示`short`，`htonl`的`l`表示`long`

> **[NOTE]** 是不是所有传入网络的数据都要手动进行转换？
> 答案是不需要。除了在`sockaddr_in`结构体填充数据的时候需要转换，我们真正发送的数据是不需要的，这个过程是自动的

## 四、网络地址的初始化与分配

### 4.1 将字符串信息转换为网络字节序的整数型

有一个接口可以将点分十进制表示的IP地址转为32位整形并且满足网络字节序，同时也支持对无效网络地址的检测：

```C
#include <arpa/inet.h>
in_addr_t   //成功返回一个大端序的整形数值；失败返回INADDR_NONE
inet_addr(const char* addr);
```

有另一个具有相同功能的接口：

```C
#include <arpa/inet.h>
int                         //成功返回1，失败返回0
inet_aton(                  
    const char* string,     //点分十进制的地址 
    struct in_addr* addr    //接受结果
);
```
还有一个功能相反的函数：

```C
#include <arpa/inet.h>
char* inet_ntoa(struct in_addr adr);
```
>**[!NOTE]**注意这里返回的是一个`char*`指针，但是没有要用户分配内存。用户使用之后应该理解将得到的结果保存到其他地址空间。

### 4.2 网络地址初始化

```C
struct sockaddr_in addr;
char* serv_ip = "211.217.168.13";
char* serv_port = "9190";
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_addr = inet_addr(serv_ip);
addr.sin_port = htons(atos(serv_port));
```

### 4.3 INADDR_ANY

这个地址的值为"0.0.0.0"，主要用在服务器绑定中。这个IP地址的含义是本机的任意IP。

### 4.4 向套接字分配网络地址

向套接字分配网络地址主要通过`bind`函数:

```C
#include <arpa/inet.h>
int                             //成功返回0，失败返回-1
bind(
    int                 sockfd, //套接字文件描述符
    struct sockaddr*    addr,   //记录IP、端口等信息
    socklen_t           addrlen //第二个参数的大小
);
```

## 五、Windows实现

大部分函数、类型在Windows平台都有类似的定义，包括

- `htons`、`htonl`
- `inet_addr`、`inet_ntoa` (Windows下没有`inet_aton`)
- `struct sockaddr_in`
- ...

此外，Windows还提供了额外的两个转换函数：

```C
#include <winsock2.h>
//将字符串转为IP地址和端口信息
INT                                         //成功返回0；失败返回SOCKET_ERROR
WSAStringToAddress(
    LPTSTR                  AddressString,  //"ip:port"格式的字符串
    INT                     AddressFamily,  //第一个参数所属的地址族
    LPWSAPROTOCOL_INFO      lpProtocolInfo, //协议提供者，默认为NULL
    LPSOCKADDR              lpAddress,      //保存地址信息的结构体地址
    LPINT                   lpAddressLength //第四个参数的结构体长度数值变量地址
)
```

```C
#include <winsock2.h>

INT
WSAAddressToString(                             //成功返回0；失败返回SOCKET_ERROR
    LPSOCKADDR          lpsaAddress,            //需要转换的地址信息结构体变量地址
    DWORD               dwAddressLength,        //第一个参数结构体的长度
    LPWSAPROTOCOL_INFO  lpProtocolInfo,         //协议提供者，默认为NULL
    LPSTR               lpszAddressString,      //保存转换结果的字符串地址值值
    LPDWORD             lpdwAddressStringLength //保存地址信息的字符串长度
)
```


