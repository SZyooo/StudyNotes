# Windows平台下线程的使用

## 一、内核对象

### 1.1 内核对象定义

操作系统创建的资源有很多种类，例如进程、线程、文件等。每个资源需要管理的数据以及管理方式会有不同。操作系统为了管理这些资源，会在内部生成数据块。每种资源需要维护的信息不同，数据的格式块也有差异。**这类数据块就被成为“内核对象”**。

### 1.2 内核对象归操作系统所有

虽然线程的资源的创建请求在进程内部完成，但是内核对象的拥有者仍然是操作系统。

## 二、基于Windows的线程创建

### 2.1 进程和线程的关系

程序开始运行之后，调用`main`函数的主体是进程还是线程？答案是线程。进程相当于装有线程的篮子，实际的运行主体仍然是线程，只不过会区分

- “单一线程模型的应用程序”
- “多线程模型的应用程序”

### 2.2 Windows下创建线程的方法

```c
#include <windows.h>

HANDLE                                              //区分线程内核对象的整形句柄
CreateThread(
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,     //线程安全相关信息，使用默认设置传递NULL
    SIZE_T                  dwStackSize,            //分配给线程栈大小，传递0生成默认大小
    LPTHREAD_START_ROUTINE  lpStartAddress,         //传递线程函数的地址
    LPVOID                  lpParameter,            //调用线程函数的参数
    DWORD                   dwCreationFlags,        //指定线程创建之后的行为，0表示立即进入可执行状态
    LPDWORD                 lpThreadId              //保存线程ID的变量地址值
);
```

Windows下的线程需要注意两个资源对象：一个是线程执行体，会在线程函数执行之后自动销毁；还有一个是线程内核对象，采用的是引用计数。初始状态下会存在两个计数，一个是线程自己，一个是返回的句柄。

虽然线程执行体本身会自动结束，但是我们需要手动`CloseHandle`来释放句柄的计数，否则内核对象会泄露。

`CreateThread`函数创建出来的线程在使用C/C++标准函数不稳定。主要原因是有一些标准函数会使用静态对象，`CreateThread`函数不会将这些对象设置为thread local存储（TLS）对象。

`CreateThread`期待的线程函数签名为：

```c
//返回值是函数返回码，通过GetExitCodeThread获取
DWORD                           
WINAPI ThreadFunction(LPVOID lpParameter);
```

为此，我们应该使用下面这个函数：

```c
#include <process.h>

uintptr_t                               //接收之后转为Handle，同样需要CloseHandle
_beginthreadex(
    void*       security,
    unsigned    stack_size,
    unsigned    (*start_address)(void*),
    void        *arglist,
    unsigned    initflag,
    unsigned    *thrdaddr
);
```
函数的参数和`CreateThread`基本上一一对应。

> **这里为什么除了句柄之外还有一个线程ID？**<br>
> 句柄的整数值在不同进程间可能出现重复，但是进程ID在跨进程范围内不会出现重复。线程ID主要用于区分操作系统创建的所有线程

## 三、内核对象的2种状态

不同内核对象具有不同信息，应用程序实现过程中需要特别关注的信息被赋予某种状态。

对于线程内核对象而言，我们特别关注的是线程是否已经终止。其中

- **signaled**：表示线程已经终止
- **non-signaled**：表示线程仍未终止

### 3.1 `WaitForSingleObject` & `WaitForMultipleObjects`

我们通过下面这两个函数来检查内核对象是否已经处于`signaled`状态：

```c
#include <windows.h>
DWORD                                   //成功返回事件信息，失败返回WAIT_FAILED
WaitForSingleObject(
    HANDLE              hHandle,        //查看状态的内核对象句柄
    DWORD               dwMilliseconds  //以1/1000秒为单位指定的超时。传递INFINITE函数不会返回，直到内核对象编程signaled状态
);
```

> 这个函数在内核对象转为`signaled`之后，有时候会将它转为`non-signaled`状态。这种支持再次进入`non-signaled`状态的对象被成为“auto reset模式”内核对象；不支持的则被称为“manual-reset模式”内核对象

```c
#include <windows.h>

DWORD                                       //成功返回事件信息，失败返回WAIT_FAILED
WaitForMultipleObjects(
    DWORD               nCount,             //需要验证的内核对象数量
    const HANDLE        *lpHandles,         //存有内核对象句柄的数组的地址
    BOOL                bWaitAll,           //是否等待全部？如果不的话，则任意内核对象转为signaled之后都会返回
    DWORD               dwMilliseconds      //超时
);
```

这两个函数在目标内核对象进入`signaled`状态之后返回`WAIT_OBJECT_0`,超时返回`WAIT_TIMEOUT`。
