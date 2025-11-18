# Soft Shadow

这一篇来讲解如何在SDF场景中添加阴影。

## 一、基本思路

在SDF场景中应用阴影的思路很简单：在每个着色点朝着光线方向进行步进，如果碰到了物体，则该着色点有阴影；否则该着色点没有阴影。

## 二、简单实现

基于上面的基本思路，我们可以很容易写出下面的代码：

```glsl
float tolerance = 1E-4;
float shadow(vec3 wPos, vec3 toLight, float maxDist, float k)
{
	float res = 1;
    //这一步很关键，因为需要阴影着色的位置都是在物体表面，因此我们需要将起始点稍微偏移出去一点（偏移量大于tolerance），不然for循环第一次得到的sd一定是小于tolerance从而退出
	float t = 0.01;
	for(int i = 0; i < MAX_NUM_STEP && t < maxDist; ++ i)
	{
		float sd = scene(wPos + toLight * t);
		if(sd < tolerance)
		{
			res = 0;
			break;
		}
		t += sd;
	}
	return res;
}

```

## 二、简单优化

这样产生的阴影是硬边的，看下图：

<image src = "../images/shadow_hard_edge.png" width = 300 style = "display: block; margin: 0 auto;">

我们可以沿着这样的思路进行优化一下：

- 1.空间一个点到最近的物体的距离$d$越近，则越“越容易”产生阴影，我们不妨称为“临近阴影”
- 2.如果产生上面的“临近阴影”的点离我们进行阴影着色的点距离$t$越远，则阴影的效果越低
  
综上，我们可以得到下面的关系：

$ shadow \propto \frac{d}{t}$

下图演示了这两个距离：

<image src = "../images/improv_1.png" height = 200 style="display:block; margin: 0 auto;">

基于此，我们引入这个关系之后，可以产生令人惊艳的柔和阴影：

```C++
//添加一个新的线性参数 k
float shadow(vec3 wPos, vec3 toLight, float maxDist, float k)
{
	float res = 1;
	//这一步很关键，因为每一个进行shadow计算的点，都是距离小于 tolerance的，进入函数的时候SDF
	float t = 0.01; 
	for(int i = 0; i < MAX_NUM_STEP && t < maxDist; ++ i)
	{
		float sd = scene(wPos + toLight * t);
		if(sd < tolerance)
		{
			res = 0;
			break;
		}
		t += sd;
		res = min(res, k * sd / t); //这里引入柔和阴影，而不是非黑即白
	}
	return res;
}
```
下面图中显示了引入上面的优化之后的阴影效果：

<image src = "../images/soft_shadow_1.png"  width=300 style="display:block; margin: 0 auto"/>

>这里的参数$k$的效果是：$k$越大，阴影越明显；我们可以将其理解为光源的大小：光源越大，软阴影就越明显。


## 三、进一步优化

上面的实现有一个问题：我们在求阴影的过程中进行步进，可能会跳过步进方向上距离障碍最近的点，下图展示了这种情况：

<image src="../images/improv_2.png" style="display:block; margin: 0 auto;" width = 250>

上图中，距离障碍最近的点应该是$O'$，但是我们步进的过程中仅仅会经过$O1$和$O2$。根据简单的几何，我们有下面的数值关系——

\[
\begin{cases}
r_1^2 - (r_1 - y)^2 = d^2\\
r_2^2 - y^2 = d^2
\end{cases}
\]

得到——

\[
\begin{cases}
y = \frac{r_2^2}{2r_1}\\
d = \sqrt{r_2^2 - y^2}
\end{cases}
\]

据此，我们可以这样优化：

```glsl
//将参数k改为w，w即solid angle，表示光源大小
float shadow(vec3 wPos, vec3 toLight, float maxDist, float w)
{
	float res = 1;
	//这一步很关键，因为每一个进行shadow计算的点，都在物体表面，即距离小于 tolerance。
	//如果不进行偏移，则循环一定会在第一次就退出
	float t = 0.1; 
	float prev_sd = 1e20;
	for(int i = 0; i < MAX_NUM_STEP && t < maxDist; ++ i)
	{
		float sd = scene(wPos + toLight * t);
		if(sd < tolerance)
		{
			res = 0;
			break;
		}
		//r1 = prev_sd
		//r2 = sd
		float y = (sd * sd) / (2.0 * prev_sd);
		float d = sqrt( sd * sd - y * y );
		res = min( res, d / (w * max(t - y, 0) ));
		prev_sd = sd;
		t += sd;
	}
	return res;
}
```
