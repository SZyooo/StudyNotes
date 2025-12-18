# Cellular Noise

细胞噪声的原理很简单：距离场（distance field）——对于每一个像素，计算其到最近特征点的距离。

最直接的计算方法：

```glsl
float m_dist = 100.;  // minimum distance
for (int i = 0; i < TOTAL_POINTS; i++) {
    float dist = distance(st, points[i]);
    m_dist = min(m_dist, dist);
}
```

但是这样做存在下面两个问题：

- 我们无法动态给出特征点数量，因为glsl语言不支持动态范围的`for`循环
- 如果顶点给的比较多，则`for`循环需要遍历的顶点过多，比较消耗性能
  
## 分格

Worley-Noise设计了这样一个算法来避免上面的两个问题：我们将空间划分为一个个网格，然后在每个网格内部生成随机点。这样一来，对于每个像素来说，它只需要计算

- 它自己所在的网格的特征点
- 以及它所在网格周围的8个网格内特征点

的距离。

这样的话，将`for`循环控制在了9次，完美解决了上面的两个问题。



