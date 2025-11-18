# 各种形状的距离场公式

## 一、基本3D形状的SDF

### 1.1 圆形 Sphere

<img src="./../images/sphere.png" width="200" height="200" alt="没有加载到图片" style="display: block; margin: 0 auto;">

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

### 1.2 盒子 Box

<img src="../images/box.png" width="200" height="200" alt="没有加载到图片" style="display: block; margin: 0 auto;">

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

<img src="../images/deduction_box.png" width="200" height="200" alt="没有加载到图片" style="display: block; margin: 0 auto;">

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

### 1.3 圆边盒子 RoundBox

<image src="../images/roundbox.png" style="display:block; margin:0 auto;">

**代码**

```glsl
float sdRoundBox(vec3 p, RoundBox b)
{
	vec3 local_pos = (b.inv_model * vec4(p, 1.0)).xyz;
	vec3 box_radius = b.radius.xyz;
	float round_radius = b.radius.w;

	vec3 q = abs(local_pos) - (box_radius - vec3(round_radius));
	return length(max(q, 0.0)) + min(0.0, max(q.x, max(q.y, q.z))) - round_radius;
}
```

如果盒子的边沿是圆弧的，计算和硬边box是类似的：我们将盒子往内挤压圆边的半径大小，然后计算和缩小的盒子的距离；这样会使得我们计算出来的距离多出圆边半径的长度，我们直接减去即可。下面的图演示了这种思路：

<image src="../images/deduction_roundbox.png" width = 300 style="display:block; margin: 0 auto;">

### 1.4 平面Plane
<figure>
<image src = "../images/plane.png" width = 150 alt = "平面" style="display: block; margin: 0 auto;">
<figcaption align="center"> 平面 </figcaption>
</figure>

**代码**

```c++
float sdPlane( vec3 p, vec3 n, float h )
{
  // n must be normalized
  return dot(p,n) + h;
}
```

**证明**

空间内经过点$P$、法线为$n$的平面的上的任意一点Q满足

\[
    S : (Q - P)\cdot n = 0
\]

简化之后就是一般式：

\[
    S : Pn + d = 0, n是平面法向量  
\]

根据投影可以证明：空间上任意一点代入到平面方程得到的值就是该点距离该平面的距离——

<image src="../images/plane_distance.png" width = 300 style="display: block; margin: 0 auto;" />

\[
    h = (Q-P)\cdot n
\]

> 这个结论放在2D平面上也是一样的：对于2D平面上的直线$Ax + By + C = 0$，任意点$p$距离该直线的距离为 $\frac{A\cdot p.x + B\cdot p.y + C}{\sqrt{A^2 + B^2}}$. 这里除以一个$\sqrt{A^2 + B^2}$是因为向量$[A, B]$虽然是直线的“法向量”，但是长度不一定是$1$。

## 二、旋转和挤出

除了直接通过3D形状计算sdf，我们还可以基于2D面片，通过旋转和挤出来构造新的形状。

### 2.1 旋转



### 2.2 挤出


# Reference

[上面方程来自这里](https://iquilezles.org/articles/distfunctions/)