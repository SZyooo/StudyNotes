# Fractal Brownian Motion

分型布朗运动（Fractal Brownian Motion， **fBM**）是一种生成随机数的算法。

##### 布朗运动 Brownian Motion， BM

描述的是一个对象的位置随着时间随机的变换，可以理解为`position += white_noise()`。形式上，布朗运动就是白噪声在时间上的积分。

##### 分型布朗运动 fBM

分型布朗运动和布朗运动类似，唯一不同的地方是每个运动之间不是完全相互独立的，而是存在某种记忆性。

如果“记忆”是正相关的（positively correlated），则某个方向的运动会导致下次运动更靠近这个方向，导致运动轨迹更平滑；如果是反相关（negatively correlated），则会导致下次运动更远离这次运动的方向，这会导致运动轨迹更加随机。

控制这种“记忆”行为的参数被称为赫斯特指数（Hurst Exponent， **H**）。这个指数会控制布朗运动的“记忆性”，进而控制其自相似性（self-similarity）、分形维数（fractal dimension）和功率频谱（power spectrum）。H的值域为$[0,1]$，值越小越粗糙、随机，值越大越平滑。普通布朗运动的H=$0.5$.

##### 自相似随机结构

自相似随机结构可以用来对很多自然景观进行建模，例如云、山等。直觉上来说，自然界中的很多大型结构可以划分为三个部分的组合：
- 大型形状勾勒出大致形状
- 中型形状对等高线和面进行扰动
- 小型形状添加细节
  
## 一、波

波（**wave**）具有以下两个特征：

- 波幅，amplitude
- 频率，frequency
  
典型的，一个简单的一维线性波可以由`y = a * sin(f * x)`描述。

<image src="../images/linear_simple_wave.png" style="display:block; margin: 0 auto">

波可以进行**叠加（superposition）**。

通过将不同的噪声叠加（称之为一个个octave），并且在这个过程增加叠加的噪声的频率（lacunarity）、减少其振幅（gain），我们可以获取一个细节更加丰富的噪声。

> 这里使用到了乐理中的名词八度octave。乐理中，两个音符之间差一个八度，则频率翻倍或者半分。类似的，这里叠加波的时候，波的频率会不断翻倍。不过频率的增加没有固定为翻倍，可以采用其他的增长方式。

## 二、基本思路

fBM的实现思路是通过一些随机函数来获取随机值，然后利用这些随机值进行自相似结构的构造。fBM的做法是，先生成一个基本的随机噪声，然后不断叠加上更小的噪声。代码上类似于：

```C++
float fbm(vecN x, float H)
{
    float t = 0.0f;
    for( int i=0; i<numOctaves; i++)
    {
        //frequency每次翻倍
        float f = pow(2.0, float(i));
        //amplitude减少
        float a = pow(f, -H);
        t += a*noise(f*x);
    }
    return t;
}

```
我们可以将上面的代码优化为，去掉指数运算：

```glsl
float fbm(vec N x, float H)
{
    float G = exp2(-H);
    float f = 1.0f;
    float a = 1.0f;
    float t = 0.0f;
    for( int i=0; i < numOctaves; ++i)
    {
        t += a * noise(f * x);
        f *= 2.0;
        a *= G;
    }
}

```

上面的代码中，每一次迭代我们都将频率翻倍，因此使用了`numOctave`这个乐理上的名词。不过这不是固定的，我们可以采用其他的增长方法。例如我们可以将`f = pow(2,i)`改为`f = 2 * i`，由指数增长改为线性增长。

## 三、另一个版本

有的实现版本没有使用到上面的赫斯特指数，而是使用了另外的两个控制参数：

- **Lacunarity** ：控制频率的增幅
- **Gain** : 控制幅度的增幅

下面是代码的实现：

```glsl
// Properties
const int octaves = 1;
float lacunarity = 2.0;
float gain = 0.5;
//
// Initial values
float amplitude = 0.5;
float frequency = 1.;
//
// Loop of octaves
for (int i = 0; i < octaves; i++) {
	y += amplitude * noise(frequency*x);
	frequency *= lacunarity;
	amplitude *= gain;
}
```

## 四、自相似性

我们前面提到过，赫斯特指数**H**控制了曲线的自相似性（静态的，即某一时刻下）。在一维的`fBM()`下，我们水平放大曲线$U$倍，我们应该在垂直方向上放大多少倍$V$，让曲线经过这样水平和垂直放大之后和以前看上去一样？

因为幅度$a=f^{-H}$，因此$f$增大$U$倍之后，$a$增大$a = (U\cdot f)^{-H} = f^{-H}\cdot U^{-H} = a \cdot U^{-H}$，即$V = U^{-H}$——即为上面代码中的`G`。

对于纯布朗运动来说，**H**的值为$0.5$，每次频率增加2倍的情况下，`G`的值为$2^{-\frac{1}{2}} = \frac{1}{\sqrt{2}}$.

对于自然过程而言，变换具有更多的记忆性，所以自相似性更加各向同性，即**H**值会更大
