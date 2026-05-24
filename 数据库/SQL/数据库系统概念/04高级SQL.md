# CH04. 高级SQL

## 一、函数和过程

### 1.1 声明和调用

返回普通类型的函数：

```sql
CREATE FUNCTION dept_count(dept_name VARCHAR(20))
RETURNS INTEGER
BEGIN
    DECLARE d_count INTEGER;
    SELECT count(*) INTO d_count
    FROM instructor
    WHERE instructor.dept_name = dept_name;
    RETURN d_count;
END;
```

返回关系的**表函数**（table function）：

```sql
CREATE FUNCTION instructor_of(dept_name VARCHAR(20))
RETURNS TABLE (
    ID VARCHAR(5),
    name VARCHAR(20),
    dept_name VARCHAR(20),
    salary NUMERIC(8,2)
)
RETURN TABLE (
    SELECT ID, name, dept_name, salary
    FROM instructor
    WHERE instructor.dept_name = instructor_of.dept_name
);
```

过程：

```sql
CREATE PROCEDURE dept_count_proc(
    IN dept_name VARCHAR(20),
    OUT d_count INTEGER
)
BEGIN
    SELECT count(*) INTO d_count
    FROM instructor
    WHERE instructor.dept_name = dept_count_proc.dept_name;
END;
```

SQL 允许多个过程或函数同名，通过参数个数或类型来区分（重载）。

### 1.2 PSM（Persistent Stored Modules）

PSM 是 SQL 的一套标准，定义了持久化存储的过程、函数、触发器等的语法规则，以及过程式代码的写法。

| 特性 | 语法 |
|------|------|
| 变量声明 | `DECLARE 变量名 类型;` |
| 变量赋值 | `SET 变量名 = 值;` |
| 复合语句 | `BEGIN ... END`，`BEGIN ATOMIC ... END` 确保作为单一事务执行 |
| `WHILE` 循环 | `WHILE 条件 DO 语句序列; END WHILE;` |
| `REPEAT` 循环 | `REPEAT 语句序列; UNTIL 条件 END REPEAT;` |
| `FOR` 循环 | `FOR 游标名 AS 查询 DO 语句序列; END FOR;` |
| 退出循环 | `LEAVE` |
| 继续循环 | `ITERATE` |
| `CASE` 语句 | `CASE WHEN 条件 THEN 结果 ... ELSE 结果 END;` |

**异常处理**：

SQL 支持定义异常条件并声明**句柄**（handler）来处理：

```sql
DECLARE out_of_classroom_seats CONDITION;
DECLARE EXIT HANDLER FOR out_of_classroom_seats
BEGIN
    ...
END;
```

- `EXIT`：执行完 handler 后退出当前复合语句
- `CONTINUE`：执行完 handler 后回到触发异常的语句下一条继续执行

使用 `SIGNAL out_of_classroom_seats` 触发异常。SQL 预定义条件包括 `SQLEXCEPTION`、`SQLWARNING`、`NOT FOUND` 等。

## 二、触发器

**触发器**（trigger）是一条在对数据库进行修改时自动执行的语句。设置触发器需满足两个要求：

- 指明触发条件：事件（`INSERT` / `UPDATE` / `DELETE`）和可选的 `WHEN` 条件
- 指明触发动作（`BEGIN ATOMIC ... END`）

### 2.1 触发器需求

典型场景：

- 往 *takes* 插入选课记录时，自动更新 *student* 的总学分
- 仓库库存低于阈值时，自动生成采购订单

### 2.2 语法

```sql
CREATE TRIGGER 触发器名
{BEFORE | AFTER | INSTEAD OF}           -- 触发时机
{INSERT | UPDATE | DELETE}              -- 触发事件
[OF 列名1, 列名2 ...]                   -- 仅 UPDATE：指定触发列
ON 表名 | 视图名
[REFERENCING
    [OLD [ROW] AS 旧行别名]
    [NEW [ROW] AS 新行别名]
    [OLD TABLE AS 旧表别名]
    [NEW TABLE AS 新表别名]
]
[FOR EACH {ROW | STATEMENT}]            -- 触发粒度
[WHEN (触发条件)]
BEGIN ATOMIC
    触发动作
END;
```

## 三、递归查询

### 3.1 问题背景

课程之间存在前置课程关系，需要找到一门课的所有直接或间接先修课程。

关系定义：`prereq(course_id, prereq_id)`

我们需要计算关系 *prereq* 的**传递闭包**（transitive closure）——即所有 `(cid, pre)` 对，其中 *pre* 是 *cid* 的直接或间接先修课程。

### 3.2 用迭代计算传递闭包

```sql
CREATE FUNCTION findAllPrereqs(cid VARCHAR(8))
RETURNS TABLE (course_id VARCHAR(8))
BEGIN
    CREATE TEMPORARY TABLE c_prereq (course_id VARCHAR(8));
    CREATE TEMPORARY TABLE new_c_prereq (course_id VARCHAR(8));
    CREATE TEMPORARY TABLE temp (course_id VARCHAR(8));

    INSERT INTO new_c_prereq
        SELECT prereq_id
        FROM prereq
        WHERE course_id = cid;

    REPEAT
        INSERT INTO c_prereq
            SELECT course_id
            FROM new_c_prereq;

        INSERT INTO temp
            SELECT prereq.course_id
            FROM new_c_prereq, prereq
            WHERE new_c_prereq.course_id = prereq.prereq_id
        EXCEPT
            SELECT course_id
            FROM c_prereq;

        DELETE FROM new_c_prereq;
        INSERT INTO new_c_prereq SELECT * FROM temp;
        DELETE FROM temp;
    UNTIL NOT EXISTS (SELECT * FROM new_c_prereq)
    END REPEAT;

    RETURN TABLE c_prereq;
END;
```

### 3.3 SQL 递归：`WITH RECURSIVE`

SQL:1999 开始支持 `WITH RECURSIVE` 子句进行有限形式的递归查询，包含两个子查询的并：

1. **基查询**（base query）：非递归部分
2. **递归查询**（recursive query）：引用递归视图本身

先修课程示例：

```sql
WITH RECURSIVE rec_prereq(course_id, prereq_id) AS (
    -- 基查询
    SELECT course_id, prereq_id
    FROM prereq
    WHERE course_id = 'CS-190'
    UNION
    -- 递归查询
    SELECT rec_prereq.course_id, prereq.prereq_id
    FROM prereq, rec_prereq
    WHERE prereq.course_id = rec_prereq.prereq_id
)
SELECT *
FROM rec_prereq;
```

#### 单调性要求

递归查询必须是**单调**的：若视图实例 V1 是 V2 的超集，则在 V1 上的递归查询结果也必须是 V2 上结果的超集。即输入更多数据不会导致已有结果被丢弃。

以下操作会破坏单调性，**禁止出现在递归查询部分**：

- 递归视图上的聚集函数
- 使用递归视图的子查询上的 `NOT EXISTS`
- 集合差（`EXCEPT`）运算

例如，查询没有输入边的节点时使用 `NOT EXISTS`：若先有边 `(1, 2)`，则输出节点 1；插入新边 `(3, 1)` 后，节点 1 因有了输入边而被剔除。新输入是原输入的超集，但输出并非超集，违反了单调性。

## 四、高级聚集特性

### 4.1 排名

传统方式根据 GPA 排名：

```sql
SELECT ID,
    (1 + (SELECT count(*) FROM student_grades B WHERE B.GPA > A.GPA)) AS s_rank
FROM student_grades A
ORDER BY s_rank;
```

性能较差，SQL 提供 `RANK()` 窗口函数：

```sql
SELECT ID, rank() OVER (ORDER BY GPA DESC) AS s_rank
FROM student_grades
ORDER BY s_rank;
```

可在同一个 `SELECT` 中使用多个窗口函数。

> 若 `PARTITION BY` 与 `GROUP BY` 同时存在，则先执行 `GROUP BY`，再在其结果上进行窗口函数运算。

其他排序函数：

| 函数 | 说明 |
|------|------|
| `PERCENT_RANK()` | 计算 `(r-1)/(n-1)`，`r` 为排名，`n` 为分区元组数（单元组时返回 `null`） |
| `CUME_DIST()` | 累计分布 `p/n`，`p` 为排序值 ≤ 当前值的元组数 |
| `ROW_NUMBER()` | 按排序顺序给每行一个唯一行号 |
| `NTILE(n)` | 将元组平均分成 `n` 个桶，返回所在桶号 |

空值处理：

```sql
SELECT ID, rank() OVER (ORDER BY GPA DESC NULLS LAST) AS s_rank
FROM student_grades;
-- 也可以使用 NULLS FIRST
```

#### 4.1.1 `LIMIT` 子句

某些数据库提供非标准扩展 `LIMIT`，与 `ORDER BY` 配合可方便地获取前 N 个结果：

```sql
SELECT ID, GPA
FROM student_grades
ORDER BY GPA
LIMIT 10;
```

### 4.2 分窗

窗口与分区的区别：窗口可以重叠，一个元组可同时存在于多个窗口中。

窗口与 `GROUP BY` 的区别：`GROUP BY` 每个分组输出一个值，而窗口每行仍输出一行，仅定义计算上下文。

基本语法：

```sql
function(exp) OVER (
    [PARTITION BY expr_list]
    [ORDER BY order_list]
    [frame_clause]
)
```

| 子句 | 说明 |
|------|------|
| `function(exp)` | 窗口函数（`ROW_NUMBER`、`RANK`、`DENSE_RANK`、`LEAD`、`LAG`、`SUM` 等） |
| `PARTITION BY` | 逻辑分区（省略时将整个结果集作为一个分区） |
| `ORDER BY` | 定义每个分区的排序顺序 |
| `frame_clause` | 在当前分区内进一步限定行的范围 |

帧子句写法：

```sql
{ROWS | RANGE | GROUPS} BETWEEN frame_start AND frame_end
```

简写形式 `{ROWS | RANGE | GROUPS} frame_start` 等价于从分区第一行到该帧起点。

| 帧模式 | 说明 |
|--------|------|
| `ROWS` | 物理行计数，如 `ROWS BETWEEN 2 PRECEDING AND 2 FOLLOWING` |
| `RANGE` | 逻辑值范围，偏移量为值，如 `RANGE BETWEEN 100 PRECEDING AND CURRENT ROW` |
| `GROUPS` | 按排序组（相同值的行），偏移量为组数 |

`frame_start` 可选值：`UNBOUNDED PRECEDING`（分区第一行）、`N PRECEDING`、`CURRENT ROW`

`frame_end` 可选值：`CURRENT ROW`、`N FOLLOWING`、`UNBOUNDED FOLLOWING`

示例（基于关系 `tot_credits(year, num_credits)`）：

```sql
-- 计算当年及前3年的平均学分
SELECT year, avg(num_credits) OVER (
    ORDER BY year ROWS 3 PRECEDING
) AS avg_total_credits
FROM tot_credits;
```

```sql
-- 计算从分区第一行到当前行的累计平均
SELECT year, avg(num_credits) OVER (
    ORDER BY year ROWS UNBOUNDED PRECEDING
) AS avg_total_credits
FROM tot_credits;
```

```sql
-- 计算前3年、当年、后2年的平均学分
SELECT year, avg(num_credits) OVER (
    ORDER BY year ROWS BETWEEN 3 PRECEDING AND 2 FOLLOWING
) AS avg_total_credits
FROM tot_credits;
```

```sql
-- 计算年份值在当前值 n-4 到 n 范围内所有元组的平均值
SELECT year, avg(num_credits) OVER (
    ORDER BY year RANGE BETWEEN 4 PRECEDING AND CURRENT ROW
) AS avg_total_credits
FROM tot_credits;
```
