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
    vec3 ap = abs(p);
    return length(min(p, b.radius) - b.radius);
}
```

**推导**

![推导](../images/deduction_box.png){width=40%}


