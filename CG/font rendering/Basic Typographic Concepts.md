# Basic Typographic Concepts

## 一、Font files，format and Information

### Font

字体（Font）就是一系列字符图像的集合，用来进行展示和打印。一个字体中的所有图像都共享了一些属性，例如外观、风格等。从印刷学（Typographic）的角度来说，字体还分为字体家族（Font family）和字体本身（Font **Faces**）

例如，“Palatino Regular”和“Palatino Italic”属于两个不同的Font Faces，但是都来自同一个Font Family。

“font”有时候在指代Font Faces和Font Family的时候会被混用。有时候会用它来代表Font Family，例如“font Courier”表示的是一个Font Family，但是在文件上，每个Font Faces是一个单独的ttf文件。例如arial.ttf是“Aral Regular”，而ariali.ttf代表的是“Arial Italic”。

一个数字字体文件有时候可能会包含多个Font Faces。每个Font Faces都包含字符图像、字符量度以及其他一些信息。不过有些格式下，一个Font Face会用多个文件来表达。例如Adobe的Type 1格式下，一个Font Face会使用一个文件来存放图像，一个文件存放字体量度。

如果一个字体文件包含多个face，则会被称呼为字体集（font collection）。

## 二、字符图像和映射

每个字符的图像被称作**glyph**。单个字符可以有多个不同的glyph；反过来，不同的字符可以有一样的glyph。

在FreeType库中：

- 一个字体文件包含一个glyph集合。每个glyph可以是一个bitmap、向量表达或者其他的格式（大多数可以缩放的字体采用的是数学表达+控制数据/程序的存储格式）。这些glyphs被按照任意顺序存储在文件中，并且通过一个glyph index来索引
- 字体文件包含一个或者多个表格，称作“character maps”（或者“charmaps”或者“cmaps”）。这个表格的作用是将字符码转化为一个glyph index。一个单独的字体文件可能含有多个charmap。例如许多OpenType字体包含一个面向Apple的charmap和一个Unicode charmap，使得它们可以在Mac和Windows上都可以使用。

## 三、字符和字体量度（font metrics）

每个glyph图像都有不同的度量（<a href = ".\Glyph Metrics.md">metrics</a>）来描述在渲染该字体的时候如何放置和管理它。一个字体的metrics关系到了glyph位置、鼠标跨度以及字符的排布。

每种可伸缩的字体格式同样包含一些全局的度量信息，用<a href = "./glyph outline.md">font units</a>来表达。

对于不可伸缩的字体，同样会包含量度。但是它们仅仅包含有限的量度，例如字符的维度和resolution，并且通常是用像素来描述。

## Reference

<a href = "https://freetype.org/freetype2/docs/glyphs/glyphs-1.html">FreeType Glyph Conventions /I</a> 
