# Glyph Outlines

## 一、像素pixel，点point和分辨率（deivce resolution）

**dpi**

"dots per inch"。每英寸的像素数量。例如对于一个dpi=300x600的打印机而言，横向每英寸可以打印300个像素、纵向每英寸可以打印600个点。

**resolution**

表示的是显示器的像素数量。例如1024x768表示的是显示器横向有1024个像素，纵向有768个像素。

可以很容易理解的是：对于相同分辨率但是大小不同的显示器，它们之间的DPI肯定有很大的不同。所以，基于这个原因，字体的大小一般不会使用pixel来表述，而是使用point来描述——

**point**

Points是一个物理单位，表示的是1/72英寸。

因此，我们可以基于一个字体的points大小计算出它的pixel大小：

$ pixel\_size = point\_size * resolution / 72 $

（注意这里的resolution表示的是dpi，例如300*200，表示的是横向和纵向的dpi。和我们一般意义上的分辨率——表示的是横向和纵向的总的像素个数——不一样）

## 二、向量表达

glyph outlines是一系列路径（又被称为“contour”）组成的。每个contour限定了glyph内部或者外部的区域。每个contour要么是二阶（conic）或者三阶（cubic）贝塞尔曲线。因此，每个glyph都是有一组点构成（注意这里的“点”不是上面的point），这些点要么是曲线控制点，要么就是普通的绝对位置点。

这些点的坐标处于一个叫做*EM sqaure*的虚拟空间。我们可以为EM Square分配一个实际的物理大小。例如我们设置EM Square大小为12pt（这里指的的是整个EM Square的大小），则一个EM Square在300x300 dpi的设备上，大小为：$300 \times\frac{12}{72} = 50$ pixels。

> 我们在Word文档中设置一个字体的大小为多少榜（pt）的时候，设置的就是这个字体整体的EM Square的大小

EM Square在unit上的大小会限定住绘制曲线的范围。例如4 units大小的EM Square，只有25个点可以使用（5 * 5）。

另外，glyph可以自由地被绘制在EM Square外面去。

EM Square units经常被称呼为**font units**或者**EM units**。

## 三、Hinting和Bitmap渲染

很显然，在将glyph的曲线渲染到屏幕上之前，需要根据EM Square大小进行缩放。缩放操作非常简单，但是在很小的大小上，会出现一些渲染瑕疵，特别是字母'E'和'T'中间的竖杠（stem）。因此，glyph渲染的时候，需要将进行缩放的点对齐到目标设备的像素网格，这中间采用的方法叫做*grid-fitting*（也叫*hinting*）。这个操作的目标是保证字体中一些重要部位的宽度和高度被特殊考虑，例如字母'I'和'T'中间的竖线。有很多方法可以实现grid fitting；大多数可以缩放的字体都会携带额外的控制数据甚至程序来实现：

- **explicit grid-fitting**
    TrueType格式定义了一个基于栈结构的虚拟机，字节码的程序可以写入到这个虚拟机中，支持200多个操作，大部分都是关于几何操作的。因此，每个glyph都由一个outline和一个控制程序组成

- **implicit grid-fitting（也叫做hinting）**
    Type 1，CFF和CFF2格式采用这种更加简单的方法：往字体文件中写入额外的描述性信息，例如字体中的stems。并没有很多的hinting类型，并且这需要基于渲染器对hinting的解析才能发挥作用。

- **automatic grid-fitting**
    有些字体仅仅由outline组成，不带任何其他的控制信息。因此完全基于渲染器来基于猜测进行grid-fitting。

## Reference

- [FreeType Glyph Conventions/II](https://freetype.org/freetype2/docs/glyphs/glyphs-1.html)
