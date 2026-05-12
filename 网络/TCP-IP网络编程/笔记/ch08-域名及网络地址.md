# 域名及网络地址

## 一、域名系统

DNS(Domain Name System)是对IP地址和域名互相转换的系统，其核心是DNS服务器。

### 1.1 什么是域名

域名是赋予某个网络主机的虚拟地址，便于用户记忆。但是访问目标主机需要将该虚拟地址修改为实际地址。

担负将这个虚拟地址转为DNS地址的是DNS服务器。

每一台计算机都记录着默认DNS服务器的地址。在浏览器输入域名之后，会通过这个默认DNS服务器获取到该域名对应的实际IP地址，从而访问该服务器。

> **[NOTE]如何获取一个域名的IP?** 我们可以通过`ping www.xxx.com`的方法来获取一个域名的IP地址

> **[NOTE]如果知道DNS服务器的地址？** 我们在Linux中，输入`nslookup`，在弹出提示信息之后输入`server`之后可以看到DNS服务器的IP。

DNS服务器无法查询到域名的IP的时候，会询问其他的DNS服务器。

## 二、IP地址和域名之间的转换

### 2.1 使用域名的必要性

IP地址更换的频率比域名更好，因此应该依赖域名而不是IP，这样的话，IP发生改变，只需要将域名映射到新的IP即可继续提供服务。


### 2.2 利用域名获取IP地址

使用以下函数来通过域名获取IP地址：

```C
#include <netdb.h>
struct hostent* 
gethostbyname(
    const char* hostname
);

```

其中，返回值结构体`hostent`的含义是“host entry”，定义如下：

```c
struct hostent
{
    char* h_name;
    char* h_aliases;
    char* h_addrtype;
    int h_length;
    char** h_addr_list;
};
```

其中每个成员的含义是：

- `h_name`：官方域名。官方域名代表的是某一个主页
- `h_aliases`：一个IP可以绑定多个域名
- `h_addrtype`：地址类型，如果是IPv4，则值为AF_INET
- `h_length`：地址变量的长度。如果是IPv4地址，则长度为4；如果是IPv6地址，则长度为16
- `h_addr_list`：以整数形式保存域名对应的IP地址。**一个域名可能会被分配多个IP**
