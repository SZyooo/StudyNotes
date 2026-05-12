# Windows下的线程同步

## 一、同步方法的分类及CRITICAL_SECTION同步

### 1.1 用户模式（User mode）和内核模式（Kernal mode）

Windows操作系统的运作方式是“双模式操作”。主要存在以下两种模式：

- **用户模式**：运行应用程序的基本模式，禁止访问物理设备，访问的内存区域也是受限的
- **内核模式**：操作系统运行的模式

> 除了Windows，现代的操作系统都是用户、内核两个模式之间切换

不过，应用程序在运行过程中也会切换到内核模式，这是因为有一些操作必须在内核模式完成。

频繁的模式切换对系统而言是一种负担。

### 1.2 用户模式下的同步 vs. 内核模式同步

用户模式下的同步不需要切换到内核模式，因此性能更高；但是功能上会存在一定局限性。

内核模式下的同步则比用户模式的同步功能更多，例如可以指定超时，防止死锁。但是相应的性能相对较差。

### 1.3 CRITICAL_SECTION同步

CRITICAL_SECTION(CS)对象有点像锁，进入临界区需要获取这把锁；离开临界区则上交CS。

> 之所以说基于CRITICAL_SECTION的同步是用户模式下的同步，是因为CRITICAL_SECTION对象不是内核对象

- **初始化**

```c
#include <windows.h>

void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
```

- **销毁**

```c
#include <windows.h>

void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
```

- **获取锁**

```c
#include <windows.h>

void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
```

- **释放**

```c
#include <windows.h>

void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
```

## 二、内核模式的同步方法

典型的内核模式同步方法有基于事件（Event）、信号量、互斥量等内核对象的同步。（因为是基于内核对象，所以是内核模式下的同步）

### 2.1 基于互斥量对象的同步

- **创建互斥量**

```c
#include <windows.h>
HANDLE CreateMutex(
    LPSECURITY_ATTRIBUTES   lpMutexAttributes, //安全相关的配置信息，默认传递NULL
    BOOL                    bInitialOwner,      //如果为TRUE，则互斥量属于创建线程，并且初始化为non-signaled状态；FALSE则创建出来的互斥量不属于任何线程，状态为signaled
    LPCTSTR                 lpName              //命名；NULL表示创建无名的互斥量对象
);
```

- **销毁互斥量**

```c
#include <windows.h>

BOOL                    //成功返回TRUE，失败返回FALSE
CloseHandle(
    HANDLE hObject
);
```

> 这个函数其实是销毁内核对象的函数

- **等待互斥量**

等待互斥量使用的是`WaitForSignelObject`。

互斥量被某一线程拥有时为non-signaled状态；释放则进入signaled状态。调用`WaitForSignelObject`的结果有两个：

1. 阻塞：目标处于non-signaled状态
2. 立即返回：目标处于signaled状态

> 互斥量是auto-reset对象：在`WaitForSignelObject`顺利返回之后自动进入non-signaled状态

- **释放互斥量**

```c
#include <windows.h>

BOOL
ReleaseMutex(
    HANDLE hMutex
);

```

###  2.2 基于信号量对象的同步

- **创建信号量**

```c
#include <windows.h>

HANDLE CreateSemaphore(
    LPSECURITY_ATTRIBUTES   lpSemaphoreAttributes,  //安全配置信息，默认传入NULL即可
    LONG                    lInitialCount,          //信号量的初始值
    LONG                    lMaximumCount,          //信号量的最大值
    LPCTSTR                 lpName                  //信号量的名字，NULL表示无名信号量
);
```

- **等待信号量**

等待信号量同样使用的是`WaitForSignelObject`。

- **释放信号量**

```c
BOOL
ReleaseSemaphore(
    HANDLE          hSemaphore,         //释放的信号量对象
    LONG            lReleaseCount,      //释放的值
    LPLONG          lpPreviousCount     //保存释放前的值；NULL表示不需要
);
```

### 2.3 基于事件对象的同步

事件对象和互斥量、信号量不同的地方在于：可以手动指定其为manual reset对象。这意味着即使`WaitForSignelObject`顺利返回了，信号量也不会进入到non-signaled的状态。

- **创建事件**

```c
HANDLE
CreateEvent(
    LPSECURITY_ATTRIBUTES   lpEventAttributes,      //安全选项，默认传入NULL
    BOOL                    bManualReset,           //是否是Manual reset模式
    BOOL                    bInitialState,          //TRUE表示创建之后是signal状态；否则是non-signaled状态
    LPCTSTR                 lpName                  //名字
);

```

- **加锁**

```c
#include <windows.h>

BOOL
ResetEvent(
    HANDLE          event
);
```

将event设置为non-signaled状态

- **解锁**

```c
#include <windows.h>

BOOL
SetEvent(
    HANDLE          event
);
```

将event设置为signaled状态
