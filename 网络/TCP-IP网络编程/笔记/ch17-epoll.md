# epoll

## 一、epoll的理解及应用

`select`是IO复用技术之一，但是它原生的性能问题导致它无法被用在需要接入上百个客户端的场景。在Linux下，我们可以使用epoll技术来替代select。


### 1.1 `select`速度慢的原因

直观上会导致`select`函数成为性能瓶颈的两个原因有：

- 每次查询都需要遍历所有的文件描述符
- 每次循环都要向函数传递需要监听的对象的信息

直观上会觉得主要的性能瓶颈在循环查询所有文件描述符上，但是实际上更大的障碍在于向操作系统传递需要监听的文件描述符对象上。

> 文件描述符的管理是操作系统的职责，因此每次注册监听都需要将数据传递给操作系统

epoll很好地解决了这个问题：我们只需要向操作系统注册一次需要监听的对象，然后操作系统在事件就绪的时候通知我们。

### 1.2 `select`函数的优点

`select`函数具有很强的可移植性，因此在接入客户端不是很多、需要考虑跨平台的时候仍然可以被认真考虑。

### 1.3 实现epoll必要的结构体和函数

使用epoll主要需要下面三个函数：

- `epoll_create`:   创建epoll对象
- `epoll_ctl`:      向空间注册或者注销文件描述符
- `epoll_wait`:     与`select`类似，等待文件描述符发生变换

> 使用`select`的情况下，我们使用`fd_set`数组来保存需要监听的描述符集合；使用epoll的情况下，我们使用`epoll_create`接口让操作系统创建空间来存放需要监听的文件描述符

表示监听事件的结构体定义为：

```c
struct epoll_event{
    __uint32_t      events;
    epoll_data_t    data;
};
typedef union epoll_data
{
    void*       ptr;
    int         fd;
    __uint32_t  u32;
    __uint64_t  u64;
}epoll_data_t;
```

我们在`epoll_ctl`中使用`epoll_event`指定监听的事件；在`epoll_wait`中传入`epoll_event`接收发生的事件。

### 1.4 `epoll_create`


```c
#include <sys/epoll.h>

int             //成功时返回epoll文件描述符，失败时返回-1
epoll_create(   
    int size    //epoll实例的大小
);
```

使用这个接口向操作系统申请epoll例程。参数`size`是向操作系统提示可能需要的内存大小。但是实际上Linux 2.6.8之后会忽略这个参数。

epoll例程也由一个文件描述符表示，关闭的时候同样使用`close`函数。

### 1.5 `epoll_ctl`

```c
#include <sys/epoll.h>

int                                 //成功返回0，失败返回-1
epoll_ctl(
    int                     epfd,   //注册监视对象的epoll例程
    int                     op,     //指定监视对象的添加、删除或者更改
    int                     fd,     //需要注册的监视对象文件描述符
    struct epoll_event*     event   //监视对象的事件类型
);
```
> 'ctl'表示"control"

对于一下这条调用语句：

`epoll_ctl(A, EPOLL_CTL_ADD, B, C)`

表示的是：向epoll例程`A`中注册文件描述符`B`，监视`C`中的事件

再如：

`epoll_ctl(A, EPOLL_CTL_DEL, B, NULL)`

表示的是：从epoll例程`A`中删除文件描述符`B`

第二个参数支持的可选项有：

- `EPOLL_CTL_ADD`: 将文件描述符注册到epoll例程
- `EPOLL_CTL_DEL`: 将文件描述符从epoll例程中删除
- `EPOLL_CTL_MOD`: 更改注册的文件描述符的关注事件发生情况

最后一个参数`event`用来指定监视的事件，典型的用例：

```c
struct epoll_event event;
event.events=EPOLLIN;
event.data.fd=sockfd;
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
```
其中，`events`成员的可选项为：

- `EPOLLIN`         : 套接字可读（有数据到达，对端正常关闭连接，监听套接字有新连接）
- `EPOLLOUT`        : 套接字可写，调用`write`/`send`不会阻塞
- `EPOLLPRI`        : 收到优先级（PRI-ority）事件（OOB事件）
- `EPOLLRDHUP`      : 对端（远程）关闭写方向的连接（RDHUP：ReaD Hang-UP）
- `EPOLLERR`        : 发生错误
- `EPOLLET`         : 以边缘触发的方式得到事件通知(ET: Edge Triggered)
- `EPOLLONESHOT`    : 发生一次事件之后，相应的文件描述符不再收到事件通知
- `EPOLLEXCLUSIVE`  : 当多个epoll实例监听同一个文件描述符，只唤醒其中一个实例

### 1.6 `epoll_wait`

```c
#include <sys/epoll.h>

int                                 //成功返回发生事件的文件描述符数量，失败返回-1
epoll_wait(
    int                 epfd,       //监听的epoll例程
    struct epoll_event* events,     //保存发生事件的事件数组地址
    int                 maxevents,  //第二个参数最多写入的事件数量
    int                 timeout     //以ms为单位的等待事件，-1表示一直等待直到发生事件
);
``` 

## 二、条件触发和边缘触发

内核通知用户事件发生的方法有两种：

- **条件触发(Level-Trigger, LT)**: 默认模式，只要文件描述符处于可以触发事件的状态，`epoll_wait`就会持续地触发<br>
优点：编程简单，非常安全，不会错过事件<br>
缺点：频繁触发事件，会增加系统开销
- 边缘触发(Edge-Triggered, ET): 只有在文件描述符地状态发生变化的那一刻通知一次，之后就不会再触发了<br>
优点：显著减少事件通知次数<br>
缺点：编程复杂，极易出错。例如为了不错过数据，在收到`EPOLLIN`事件的时候，需要循环调用`read`/`recv`函数直到读完所有数据。

### 2.1 边缘触发的核心要点

**边缘触发的套接字必须是非阻塞的**。原因很简单：事件发生之后，我们必须立即将所有数据读取出来，否则如果没有下次触发的话，会有数据丢失；如果我们采用的是阻塞模式，在读取完了所有数据之后，我们会阻塞在`read`函数上。

我们可以通过下面的函数来修改文件描述符的属性：

```c
#include <fcntl.h>

int                         //成功返回cmd参数相关的值，失败返回-1
fcntl(
    int     file_des,       //需要修改的文件描述符
    int     cmd,            //表示函数调用的目的
    ...                     //可变参数。如果是获取属性，则传入接收属性的变量
);
```

这个函数的使用场景很广泛，这里仅介绍如何设置其阻塞：

```c
//这里传入第三个参数是约定俗成的写法:历史代码和头文件的声明要求必须提供第三个参数，所以常见做法是显示传递一个0
int flag=fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flag | O_NONBLOCK);
```
> `F_GETFL`表示的是Get Flag

在非阻塞的文件描述符上调用`read`的话，如果输入缓冲没有数据，则立即返回-1，并且将全局的错误变量`errno`置为`EAGAIN`。

因此，开发范式为：我们在边缘触发的`epoll_wait`返回之后，一直调用`read`，直到返回-1，检查`errno`。
