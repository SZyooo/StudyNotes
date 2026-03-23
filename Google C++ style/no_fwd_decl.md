### 使用前置声明的好处：

- **减少编译时间**。`#include`要求编译器打开文件并处理，因此过多的话会减慢编译时间
- **避免重复编译**。头文件里面无关的修改会导致包含它的头文件重新编译

### 使用前置声明的坏处：

- 前置声明会隐藏依赖
- 前置声明会被库修改破坏:
    - 如果库仅仅是将参数的从`short`改为`long`，直接`include`的话不需要任何修改，前置声明需要
    - 如果库给函数添加了带默认参数值的额外参数，直接`include`不需要任何修改，前置声明需要
    - 如果库将定义移动到了新的命名空间，直接`include`不需要任何修改，前置声明需要
- 对`std::`空间的前置声明符号是未定义行为
- 使用前置声明可能会导致代码的语义发生变化: 
    ```C++
    //b.h
    struct B{};
    struct D : B{}
    ```

    ```C++
    //bad_user.cpp
    struct B;
    struct D;                           //采用前置声明
    void f(B*){ printf("f(B*)");}
    void f(void*){printf("f(void*)");
    void test(D* x) { f(x); }           //调用 f(void*)
    ```

    ```C++
    //good_user.cpp
    #include <b.h>                      //直接include
    void f(B*){ printf("f(B*)");}   
    void f(void*){printf("f(void*)");
    void test(D* x) { f(x); }           //正确调用 f(B*)
    ```
- 文件中太多的前置声明会显得很冗余（Verbose）
- 为了让代码可以使用前置声明，我们需要额外处理代码。例如我们必须使用指针或者引用，而不能直接使用对象
