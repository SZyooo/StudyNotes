# 各种形状的距离场公式

## 圆形 Sphere

![sphere](./../images/sphere.png)

**代码**

```C++
struct sphere{
    vec3 center;
    float r;
}
float sdSphere(vec3 p, sphere s)
{
    return length(p - s.center) - r;
}
```

**推导**

（略）

## 盒子 Box

![box](../images/box.png)

**代码**

```C++
//在原点
struct box{
    vec3 radius; //x, y, z三个轴上的半径（注意是半径，不是直径）
}

float sdBox(vec3 p, box b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0)) + min(max(q.x, q.y, q.z), 0);
}
```

**推导**

<img src="../images/deduction_box.png" width="200" height="200" alt="图片加载失败时的提示文字">

以上图中的2D box为例，我们可以简单将2D情形推广到3D。

我们可以将任何点都对称到第一象限，因此我们这里仅仅讨论第一象限的情况。

第一象限中，点p会有四种情况，如上图所示。下面分别列出：

* 在box外面
    - 在$p_1$位置：`d=length(vec2(0, p_1.y-r.y))`
    - 在$p_2$位置：`d=length(p_2 - r)`
    - 在$p_4$位置：`d=length(vec2(p_4.x-r.x, 0))`
    可以用一个公式概括：`length(max(p-d, 0))`.其中`max(p-d, 0)`表示将`p-d`的所有分量和0进行取最大值
* 在box里面
    - 距离任意一条边最近的距离：`d = min(max(p - r), 0)`.其中`max(p-r)`表示的是`p-r`的`x`和`y`中的最大值。和0进行比较是为了过滤掉p点在外面的情况——只要p点在外面，则`x`或者`y`肯定有大于0的，和0比较之后会将距离截断为0

我们将上面的两个距离综合起来。可以使用加法：

<center>
<code>d = length(max(p-d), 0) + min(max(p-r),0)</code>
</center>


如果`p`在box内部，则第一项为0；如果`p`在box外部，则第二项为0.

