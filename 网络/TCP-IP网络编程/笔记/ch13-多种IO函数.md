# 多种I/O函数

## 一、`send`和`recv`

### 1.1 Linux下的`send`和`recv`

```c
#include <sys/socket.h>

ssize_t                         //成功返回传输的字节数
send(
    int             sockfd,     //发送数据的套接字
    const void*     buf,        //数据缓冲
    size_t          nbytes,     //需要传输的字节数
    int             flags       //指定的可选项信息
);
```

常见的可选项有：

|可选项|含义|send|recv|
|:-:|:-|:-:|:-:|
|`MSG_OOB`|传输带外数据（Out-of-band data）|+|+|
|`MSG_PEEK`|验证输入缓冲中是否存在接收的数据||+|
|`MSG_DONTROUTE`|数据传输过程中不参照路由表，在本地网络中寻找目的地|+||
|`MSG_DONTWAIT`|调用I/O函数的时候不要阻塞，用于使用非阻塞I/O|+|+|
|`MSG_WATIALL`|防止函数返回，直到接收全部请求的字节数||+|

### 1.2 `MSG_OOB`: 发送紧急消息

`MSG_OOB`表示传输“带外数据”，主要用于TCP协议，提供一种“紧急”或者“高优先级”的数据传输机制。

> **带外数据** TCP流中，正常情况下数据按照顺序传输。带外数据是插入到正常数据中的一个额外数据。**它仍然按照正常的方法发送，和其他数据一样**。关键不同的地方在于接收方可以提前将之取出先处理。<br>

TCP通过紧急指针（Urgent Pointer）来标记数据流中的带外数据。携带带外数据的TCP包头会将`URG`位置1，同时让`URG pointer`指向带外数据的下一个字节<br>

![URG MSG](./urg_msg.png)<br>

带外数据长度固定为1个字节

#### 接收紧急消息

紧急消息到来的时候，我们可以通过`select`监听异常组或者`SIGURG`信号来快速处理。

紧急消息本身只有一个字节，意味着它其实无法传递多少有效信息。它更大的意义是传递一个紧急信号，类似于`Ctrl+C`，接收端收到之后可以终止某个长时间的操作，例如终止文件传输。

**紧急消息会被覆盖**。如果连续发送两次紧急消息，则第二次紧急消息会将第一次紧急消息覆盖掉。

### 1.3 检查输入缓冲

`MSG_PEEK`选项调用`recv`函数的话，即使读取了输入缓冲的数据也不会删除；`MSG_DONTWAIT`标志下的`recv`则会以非阻塞的方法来调用IO。因此，我们可以结合这两个标志来检查输入缓冲区是否存在新数据。


## 二、`readv`和`writev`函数

### 2.1 使用`readv`和`writev`函数

这两个函数的功能概括如下：

“对数据进行整合传输及发送的函数”

即，`writev`函数可以将分散保存在多个缓冲中的数据一并发送；`readv`函数可以将收到数据接收到多个缓冲中。

使用这两个函数可以减少I/O函数调用次数。

```c
#include <sys/uio.h>                //uio: User I/O

ssize_t                             //成功返回发送的字节数，失败返回-1
writev(
    int                 filedes,    //传输数据的套接字文件描述符
    const struct iovec* iov,        //结构体数组，描述多个数据缓冲
    int                 iovcnt      //第二个参数数组的长度
);

struct iovec
{
    void*   iov_base;
    size_t  iov_len;
};

```

```c
#include <sys/uio.h>

ssize_t                             //成功返回接收的字节数，失败返回-1
readv(
    int                 filedes,    //传递数据的文件描述符
    const struct iovec* iov,        //缓冲数组
    int                 iovcnt      //第二个参数的数组长度
);
```

> Windows下没有`readv`和`writev`对应的实现，但是可以使用重叠IO
