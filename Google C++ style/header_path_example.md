
对于这个头文件，`google-awesome-project/src/base/logging.h'，我们应该这样包含它：

```C++
#include "base/logging.h"
```

对于源文件`dir/foo.cpp`，`dir/foo_test.cpp`，它们内部应该这样包含头文件：

```C++
#include <dir2/foo.h>
//2. C系统库头文件
//3. C++系统库头文件
//4. 其他库头文件
//5. 项目的其他头文件
```
