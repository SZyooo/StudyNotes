## C++的命名空间

### 一、具名命名空间

```C++
namespace A{...}
```

### 二、`inline`命名空间

```C++
inline namespace A{...}
```

`inline`命名空间内声明的对象是在父`namespace`范围内的。一个很大的作用是用作版本管理：

```C++
namespace A{
    inline namespace new_version{
        void f(){
            ...
        }
    }
    namespace old_version{
        void f(){

        }
    }   
}

void main(){
    A::f();                 //正常调用无需指定命名空间
    A::old_version::f();    //显式调用旧版本
}

```

### 三、匿名命名空间

```C++
namespace{
    //...
}
```

匿名名称空间内的成员具有`internal linkage`，在整个翻译单元都是可见的。用来代替`static`关键字的作用。

### 四、重命名命名空间

```C++
namespace new_name = std;
```

### 五、嵌套命名空间

```C++
namespace A::B{}
/*
等价于：
namespace A{
    namespace B{
        //...
    }
}
*/
```

### 六、嵌套`inline`命名空间

```C++
namespace A::inline B{}
/*
等价于：
namespace A{
    inline namespace B{
        //...
    }
}
*/
```

### 七、使用命名空间的优缺点

#### 优点
- 避免名字冲突

#### 缺点
- 命名空间会增加复杂度
- `inline`命名空间会让增加名字的困惑性：一个名字并没有被限定在它所在名称空间可见
- 在头文件中使用匿名名称空间会隐性破坏ODR（One Definition Rule）：
    ```C++
    // config.h
    namespace {
        std::string config = "default";
    }

    void set_config(const std::string& s) {
        config = s;  // 修改的是当前编译单元的config
    }

    // a.cpp
    #include "config.h"
    // 调用 set_config("test");

    // b.cpp
    #include "config.h"
    // 打印 config → 结果还是 "default"！
    ```
- 重复地引用同一个命名空间的名字会使得书写变得啰嗦
