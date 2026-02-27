# Glyph Metrics

## 一、Baseline，pens and layouts

baseline是一条想象中的线，用来指导glyphs怎么进行渲染。这条baseline可以是水平的，**也可以是垂直的**。

另外，一个在baseline上的虚拟点，被称作**“pen position”**或者**“origin”**，被用来定位glyph。

不同的布局（layout）使用不同的习惯来放置glyph：

- 水平布局下，glyphs被简单地放置在baseline上。文本通过递增pen position（从左到右或者反过来）进行放置。
两个相邻的pen position之间的距离由glyph来决定，被称作 <b>“advance width” </b> 。注意，这个值永远是正的，即使是从右到左进行渲染。

pen position永远位于baseline上：

<img src=".\\images\\metrics01.png" alt="" style="display: block; margin: 0 auto; width: 300px;">

- 垂直布局下，glyphs被放在baseline的中心：

<img src=".\\images\\metrics02.png" alt="" style="display: block; margin: 0 auto; width: 150px;">


## 二、印刷学量度和包围盒

一个face可以拥有不同的metrics：

- **Ascent**
    放置一个outline点（控制点或者绝对坐标点）的最高网格坐标（EM Space）

- **Descent**
    放置一个outline点（控制点或者绝对坐标点）的最低网格坐标（EM Space）

- **Linegap**
    行间距，一个额外的距离值。两行文本之间的最小距离。添加上行间距之后，一行文本的实际距离为：
    \[linespace = ascent - descent + linegap\]

除此之外，还有其他的metrics：

- **Bounding Box**
    将每个glyph包围的包围盒。使用`xMin`，`xMax`，`yMin`，`yMax`来表示。

- **Internal leading**
    内部行距。计算公式是：
\[internal leading = ascent - descent - EM_{size}\] 
    它的含义是：除去glyph本身需要的大小，额外给它留出来的上下空间

- **External Leading**
    linegap的别名

## 三、Bearing and Advances

每个glyph还有两个额外的距离，_bearing_和_advances_。这两个距离的实际值根据实际的水平或者垂直布局来：

- **Left side bearing**
    
    当前pen position到glyph的bbox左边界的距离。大部分情况下，水平布局的时候值是正的，垂直布局的时候值是负的。
    在FreeType API中，这个距离的名字叫做`bearingX`。
    有时候这个值被简称为'lsb'。

- **Top side bearing**
    从baseline到glyph的bbox的顶部的距离。对于水平布局，这个值一般是正的；对于垂直布局，这个值是负的。
    在FreeType API中，这个距离的名字叫做`bearingY`。

- **Advance width**
    垂直布局下，渲染一个字体之后递增（从左到右渲染）/递减（从右到左渲染）pen position的距离。
    在FreeType API中，这个距离的名字为`advanceX`。

- **Advance height**
    水平布局下，渲染一行字体之后递增（从上到下渲染）/递减（从下到上渲染）pen position的距离值。
    在FreeType API中，这个距离的名字为`advanceY`

- **Glyph width**
    glyph的水平宽度。在没有缩放的情况下，计算很简单：
    \[glyph width = bbox.xMax - bbox.xMin\]
    对于有缩放的glyph，则需要特别小心（特别是grid fitting）。

- **Glyph height**
    glyph的垂直高度。在没有缩放的情况下，计算很简单：
    \[glyph height = bbox.yMax - bbox.yMin\]
    对于有缩放的glyph，则需特别小心。

- **Right side bearing**
    和Left side bearing是相对的概念，表示的是包围盒有边界到advance width的距离：
    \[right side bearing = advance\_width - left\_side\_bearing - (xMax - xMin)\]
    有时候这个值被简称为'rsb'。

下图中，给出了上面一系列参数的图示：


<img src=".\\images\\metrics03.png" alt="" style="display: block; margin: 0 auto; width: 300px;">


<img src=".\\images\\metrics04.png" alt="" style="display: block; margin: 0 auto; width: 300px;">

## 四、grid fitting效果

[hinting（又叫grid-fitting）](.\\glyph outline.md)会将glyph的控制点对齐到像素网格，这个操作会细微地改变字符图像。

例如，小写字母'm'有时候在EM square中是一个正方形，但是为了让它在很小的像素大小下可读，hinting会刻意将它的outline在横向上进行拉伸，从而保证它的三条“腿”可见；这样很显然会导致更宽的bitmap。

glyph的metrics同样会被grid-fitting过程影响：

- 图像的宽度和高度被修改。虽然只有一个像素大小，但是在很小的像素大小的时候也是很明显的变换

- 图像的包围盒被修改，从而修改了bearing值

- advances必须被更新。例如，被hinted的bitmap如果变大了，则advance width必须增大

下面是一些需要注意的地方：

- 因为hinting的存在，简单的缩放ascent或者descent可能会导致错误的结果。一种解决办法是使用缩放之后ascent的ceiling，缩放之后的descent的floor值。

- 没有一个简单的方法来批量获取被hinted的glyphs以及advance widths。hinting在不同的glyph上有不同的效果，因此必须一个glyph一个glyph的进行操作。


使用Free Type库对glyph进行2D位移是很简单的操作。但是要注意的是，如果一个glyph被hinted了，则再进行位移操作的时候，应该始终移动整数像素的位置，否则会破坏掉hint的效果。

>在FreeType中，通过API`FT_Outline_Translate(signed long)`来移动字体位置。有意思的是，其中的参数是一个`long`类型，长度为32bit。FreeType有时候会将这32bit解释为`int`，有时候会解释为16.16格式的浮点数，或者26.6格式的浮点数。在这个API中，FreeType将它解释为26.6格式的浮点数，这意味着末尾6位是小数。为了避免出现小数，我们传入的值要保证后面6位都是0，这意味着值应该是$2^6 = 64$的整数倍。例如，我们希望移动2个单位，则这样`FT_Outline_Translate(2 * 64)`。


## 文本宽度和包围盒

根据前面的描述，我们知道glyph的原点是baseline上pen position的位置。不过这个pen position/origin没必要一定是包围盒的顶点。有时候它可以在包围盒的外面或者里面。

类似的，glyph的‘advance width’是绘制下一个glyph的时候pen position的增加，和glyph本身的宽度没有直接关系——后者是glyph的包围盒的宽度。

当渲染文本的时候是类似的：

- 文本串的boundingbox不一定会将末尾的光标包含进去
- 文本串的advance width和它的boundingbox也没有什么关系，特别是在加入了前缀或者后缀空格之后
- 一些额外的操作（诸如[kerning](Kerning.md)）创造出来的文本串在度量数值上不是单独的glyph的度量数值的叠加。例如，文本“VA”的advance width并不是两个字符‘V’和‘A’的advance width的叠加。

## Reference 

<a href="https://freetype.org/freetype2/docs/glyphs/glyphs-3.html">FreeType Glyph Conventions /III</a>
