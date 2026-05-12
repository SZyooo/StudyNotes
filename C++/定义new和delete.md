# 定制`new`和`delete`

## 一、`new`关键字的三重含义

C++中，`new`关键字有下面三重含义：

- **`new` operator(`new`操作符)**：就是我们日常写的`A* a = new A;`，主要完成两件事：
    - 分配内存
    - 调用构造函数

- **operator `new`函数**：仅仅分配内存

- **placement `new`**：除了`size_t`参数之外还有其他额外参数的operator `new`。

> 其实本质上只有两层含义：new对象时候是new operator，在new operator生效的期间会调用operator new函数。
## 二、new-handler

标准库提供的分配内存的operator `new`的行为概括如下：

```c++
void* operator new(std::size_t size) throw(std::bad_alloc)
{
    using namespace std;
    if(size=0)
        size= 1;//要求0的话也得返回1
    while(true)
    {
        尝试分配内存
        if(分配成功)
            return 内存指针
            new_handler gHandler = set_new_handler(nullptr);//获取new-handler
            set_new_handler(new_handler);

            if(gHandler)
                gHandler();
            else throw std::bad_alloc{};
    }
}
```

可以看到，如果operator `new`分配内存失败的话，在抛出`std::bad_alloc`之前会尝试调用一个`std::new_handler`函数。我们可以通过`std::set_new_handler`来设置一个自定义的new handler：

> 这里调用的operator `new`指的是标准库提供的单参数`size_t`的默认版本

```c++
namespace std
{
    typedef void (*new_handler)();
    new_handler set_new_handler(new_handler p) throw();
};
```

我们如果尝试安装自己的`new_handler`的话，需要在`new_handler`内至少做下面四件事之一：

- **让更多内存可以使用**:空出内存
- **安装另一个`new_handler`**:如果直到有其他的`new_handler`可以有所作为，则将它安装
- **卸载`new_handler`**:没有`new_handler`的情况下，默认的operator `new`会抛出异常
- **抛出`std::bad_alloc`或者其派生类的异常**
- **直接退出`exit`或者`abort`**

## 三、operator `new`

operator `new`本质上就是一个函数。我们写下这样的代码：

```c++
A* a = new A;
```
的时候，**调用了`new`操作符**（即`new operator`）。调用`new`操作符的结果是触发两件事：

1. 调用operator `new`申请内存
2. 在operator `new`返回的地址上调用目标的构造函数

所谓的operator `new`就是第一步会被调用的函数。

我们可以重载全局作用域的operator `new`，也可以重载单独类的operator `new`(**这要求类的operator `new`必须是`static`的**)。

标准库提供了默认的三个operator `new`重载:

```c++
void* operator new(std::size_t) throw(std::bad_alloc);
void* operator new(std::size_t, const std::nothrow_t&) throw();
void* operator new(std::size_t, void*) throw();
```
我们一般情况下写的代码：

```c++
A* a = new A;
```
调用的是第一个重载，如果分配失败会抛出异常（在那之前会调用new_handler)。

我们可以这样调用第二个重载：

```c++
A* a = new (std::nothrow) A;
```

第三个重载版本的实现很简单，不分配任何内存，只是将指针参数直接返回。主要作用是在指定内存上构造对象：

```c++
A* pa = malloc(A);
pa = new(pa)A;
```

## 四、placement `new`

什么是placement `new`? 答案很简单：**<font color="red">带有除了第一个`size_t`参数的operator `new`就是placement `new`</font>**。

因此标准库提供的第2、3个operator `new`都是placement `new`。

## 五、placement `delete`

和`new` operator一样，我们写下这样一行代码：

`delete pa;`

的时候，其实是调用`delete` operator。类似的，`delete` operator主要执行了下面两个动作：

1. 在内存上调用析构函数
2. 执行operator `delete`函数释放内存

**<font color="red">标准要求：使用哪个版本的operator `new`，则必须调用参数一致的operator `delete`。如果找不到对应参数版本的operator `delete`，则不会执行任何operator `delete`</font>**

