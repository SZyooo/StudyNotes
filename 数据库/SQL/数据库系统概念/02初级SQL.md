# CH02. SQL

## 一、SQL查询语言概览

SQL 最早由 IBM 开发，最初被称为 Sequel。发展至今，名称已演变为 SQL（结构化查询语言）。

ANSI 和国际标准化组织（ISO）发布了 SQL 标准，最新版本为 SQL:2008。

SQL 主要包含：

- **数据定义语言**（Data-Definition Language，DDL）：定义关系模式、删除关系以及修改关系模式的命令
- **数据操纵语言**（Data-Manipulation Language，DML）：从数据库查询信息，以及插入、删除、修改元组
- **完整性**（Integrity）：DDL 中包含定义完整性约束的命令
- **视图定义**（view definition）
- **事务控制**（transaction control）
- **嵌入式 SQL 和动态 SQL**：定义 SQL 语句如何嵌入通用编程语言（如 C、C++、Java 等）
- **授权**（authorization）：定义对关系和视图的访问权限

## 二、SQL数据定义

数据库中的关系集合必须由数据定义语言（DDL）指定给系统。DDL 可定义如下信息：

- 每个关系的模式
- 每个属性的取值类型
- 完整性约束
- 每个关系维护的索引集合
- 每个关系的安全性和权限信息
- 每个关系在磁盘上的物理存储结构

这里仅讨论模式定义和基本类型。

### 2.1 基本类型

SQL 支持如下固有类型：

| 类型 | 说明 |
|------|------|
| `char(n)` | 固定长度字符串 |
| `varchar(n)` | 可变长度字符串，最大长度 n |
| `int` | 整数类型 |
| `smallint` | 小整数类型 |
| `numeric(p, d)` | 定点数，共 p 位数字（含小数点），其中 d 位小数 |
| `real` / `double` | 单精度 / 双精度浮点数 |
| `float(n)` | （二进制下）精度至少为 n 位的浮点数 |

`char` 是固定长度字符串，若给定字符数量不够会补空格。

### 2.2 基本模式定义

#### 2.2.1 定义关系

```sql
CREATE TABLE name (
    A_1 D_1,
    A_2 D_2,
    ...,
    A_n D_n,
    <完整性约束1>,
    ...,
    <完整性约束n>
);
```

其中：

- `A`：属性名
- `D`：属性类型及可选的属性约束

SQL 支持多种完整性约束，常用三种：

| 约束 | 说明 |
|------|------|
| `PRIMARY KEY (A1, A2, ...)` | 声明属性共同构成主码，**主码属性必须非空且唯一** |
| `FOREIGN KEY (A1, A2, ...) REFERENCES r` | 声明外码，取值必须对应关系 r 中某个元组 |
| `NOT NULL` | 该属性不允许出现空值 |

#### 2.2.2 删除关系

```sql
DROP TABLE r;
```

#### 2.2.3 删除属性

```sql
ALTER TABLE r ADD A D;
```

#### 2.2.3 修改属性

```sql
ALTER TABLE r DROP A;
```

### 2.3 SQL查询基本结构

SQL 查询的基本结构由三个子句构成：`SELECT`、`FROM` 和 `WHERE`：

```sql
SELECT A1, A2, ...
FROM r1, r2, ...
WHERE Pred;
```

#### 2.3.1 单关系查询

单关系查询指 `FROM` 子句后只出现一个关系。

##### 2.3.1.1 去重

关系模型中关系是集合，不允许重复元组。但去重是耗时操作，因此默认 SQL 允许查询结果出现重复。若要强制去重，在 `SELECT` 后加 `DISTINCT`：

```sql
SELECT DISTINCT dept_name
FROM instructor;
```

若要显式保留重复（默认行为），在 `SELECT` 后加 `ALL`：

```sql
SELECT ALL dept_name
FROM instructor;
```

##### 2.3.1.2 结果临时运算

SQL 支持在 `SELECT` 子句中对结果属性执行 `+`、`-`、`*`、`/` 算术表达式：

```sql
SELECT salary * 1.1
FROM instructor;
```

##### 2.3.1.3 `WHERE` 子句

`WHERE` 支持对结果元组进行过滤，支持 `AND`、`OR`、`NOT` 逻辑运算。

#### 2.3.2 多关系查询

典型的多关系查询：

```sql
SELECT A1, A2, ..., An
FROM r1, r2, ..., r3
WHERE Pred;
```

三个子句的执行顺序为 `FROM` → `WHERE` → `SELECT`：

1. 对 `FROM` 子句列出的关系生成笛卡尔积
2. 用 `WHERE` 子句的谓词过滤笛卡尔积
3. 对过滤后的关系的每个元组，输出 `SELECT` 指定的属性

#### 2.3.3 自然连接

自然连接与笛卡尔积不同：它只将两个关系中**共同属性取值相同**的元组配对连接；连接结果中共同属性只出现一次，然后是第一个关系的独有属性，最后是第二个关系的独有属性。

等价写法：

```sql
SELECT 共同属性列表, 只出现在r1的属性, 只出现在r2的属性
FROM r1, r2
WHERE r1.共同属性 = r2.共同属性;
```

SQL 支持用 `NATURAL JOIN` 替代 `,` 表示自然连接：

```sql
SELECT A1, A2, ...
FROM r1 NATURAL JOIN r2 NATURAL JOIN r3 ...
WHERE Pred;
```

若只需在指定属性上进行自然连接，可使用 `JOIN ... USING`：

```sql
SELECT A1, A2, ...
FROM (r1 JOIN r2 USING (A1, A2)) NATURAL JOIN r3
WHERE Pred;
```

#### 2.3.4 附加的基本运算

##### 2.3.4.1 更名运算

```sql
old_name AS new_name
```

更名运算可出现在 `SELECT` 和 `FROM` 中。

##### 2.3.4.2 字符串运算

SQL 中字符串可用单引号或双引号。

不同 SQL 实现提供不同的字符串函数，常见的有 `UPPER(s)`、`LOWER(s)`、`TRIM(s)` 等。

SQL 支持 `LIKE` 操作符进行模式匹配，包含两个通配符：

- `%`：匹配任意子串
- `_`：匹配单个字符

可自定义转义字符，将通配符作为普通字符使用：

```sql
LIKE 'ab\%cd%' ESCAPE '\'
```

上述语句将 `\` 作为转义字符，匹配以 `ab%cd` 开头后接任意子串的字符串。

##### 2.3.4.3 所有属性

`*` 可用于 `SELECT` 中表示所有属性。

##### 2.3.4.4 排序

使用 `ORDER BY attrib DESC|ASC` 子句定义元组显示顺序：

```sql
SELECT *
FROM r
ORDER BY r.A DESC;
```

##### 2.3.4.5 `BETWEEN` 谓词

用 `BETWEEN A AND B` 更直观地表达范围：

```sql
SELECT name
FROM instructor
WHERE salary BETWEEN 90000 AND 100000;
```

##### 2.3.4.6 临时元组

SQL 允许使用 `(v1, v2, v3, ...)` 构建临时元组，可用于比较，也可与 `IN` 操作符结合：

```sql
WHERE (A1, A2) IN (SELECT ...);
```

#### 2.3.5 集合运算

SQL 中 `UNION`、`INTERSECT` 和 `EXCEPT` 对应于数学集合论中的并、交、差运算。

所有集合运算默认去重，可在运算符后加 `ALL` 取消去重（`UNION ALL`、`INTERSECT ALL`、`EXCEPT ALL`）。

#### 2.3.6 空值

- **算术表达式中任意运算数为空，结果为空**
- **比较运算符中出现空值时，若无法因短路判断出真假，则结果为 `unknown`**

`unknown` 是除 `true`、`false` 外的第三个逻辑值。若 `WHERE` 子句判断结果为 `unknown`，则该元组不会被添加到结果中。

可使用 `IS unknown` 和 `IS NOT unknown` 判断逻辑值是否为 `unknown`。

使用 `SELECT DISTINCT` 去重时，空值被认为相等。例如 `(A, null)` 和 `(A, null)` 被视为相等。但在谓词中 `null = null` 返回 `unknown`，而非 `true`。

#### 2.3.7 聚集函数

SQL 提供五个固有聚集函数：

| 函数 | 说明 |
|------|------|
| `avg` | 平均值 |
| `min` | 最小值 |
| `max` | 最大值 |
| `sum` | 总和 |
| `count` | 计数 |

> 标准还定义了 `some` 和 `every`，但未广泛实现。

聚集函数主要用在 `SELECT` 子句中。

##### 2.3.7.1 基本聚集

直接在查询结果上使用聚集函数：

```sql
SELECT avg(salary)
FROM instructor
WHERE dept_name = 'Comp.Sci';
```

**去重**：在聚集运算对象前加 `DISTINCT` 表示先去重再聚集（`ALL` 为默认行为）：

```sql
SELECT count(DISTINCT ID)
FROM teaches;
```

SQL 不允许在 `SELECT count(*)` 中使用 `DISTINCT`。

##### 2.3.7.2 分组聚集：`GROUP BY`

有时需先按属性分类再在每个分类上聚集，使用 `GROUP BY` 子句。`GROUP BY` 后给出属性集合，该集合取值完全相同的元组归为一组，聚集函数在每个组级别上进行运算。

> 注意：出现在 `SELECT` 中且未被聚集的属性，只能是 `GROUP BY` 子句属性集合中的属性。

##### 2.3.7.3 `HAVING` 子句

分组后需过滤组时使用 `HAVING` 子句（类似 `WHERE`，但作用对象是分组）：

```sql
SELECT dept_name, avg(salary) AS avg_salary
FROM instructor
GROUP BY dept_name
HAVING avg(salary) > 42000;
```

与 `SELECT` 相同，`HAVING` 中未被聚集的属性必须出现在 `GROUP BY` 中。

##### 2.3.7.4 运算顺序

含聚集的完整查询语句结构：

```sql
SELECT ...
FROM ...
WHERE ...
GROUP BY ...
HAVING ...;
```

执行顺序：

1. `FROM` 计算得到一个关系
2. `WHERE` 对元组进行过滤
3. `GROUP BY` 对过滤后的元组进行分组
4. `HAVING` 对组进行过滤
5. `SELECT` 展示指定的属性

##### 2.3.7.5 如何处理空值

聚集函数处理空值的原则：**除 `count` 外，其余聚集函数均忽略输入集合中的空值**。

若输入聚集函数的集合为空集：**`count` 输入空集得 0，其他聚集函数输入空集得空**。

##### 2.3.7.6 `some` 和 `every`

SQL:1999 引入的处理布尔值集合的聚集函数，未广泛实现。

#### 2.3.8 嵌套子查询

子查询是嵌套在另一个查询的 `SELECT-FROM-WHERE` 表达式。

##### 2.3.8.1 集合成员资格

`IN` / `NOT IN` 测试元组是否是集合的成员：

```sql
SELECT DISTINCT course_id
FROM section
WHERE semester = 'Fall' AND year = 2009
    AND course_id IN (
        SELECT course_id
        FROM section
        WHERE semester = 'Spring' AND year = 2010
    );
```

<details>
<summary><code>IN</code>/<code>NOT IN</code> 用于枚举集合</summary>
<code>WHERE name NOT IN ('Mozart', 'Einstein')</code>
</details>

##### 2.3.8.2 集合的比较

`> SOME` 表达"至少比某一个大"：

```sql
SELECT name
FROM instructor
WHERE salary > SOME (
    SELECT salary
    FROM instructor
    WHERE dept_name = 'Biology'
);
```

类似地，还有 `<SOME`、`<=SOME`、`>=SOME`、`=SOME`、`!=SOME`，以及对应的 `ALL` 版本（`<ALL`、`<=ALL` 等）。

##### 2.3.8.3 空关系测试

`EXISTS` 用于判断子查询结果是否非空：

```sql
SELECT course_id
FROM section AS S
WHERE semester = 'Fall' AND year = 2009
    AND EXISTS (
        SELECT *
        FROM section AS T
        WHERE semester = 'Spring' AND year = 2010
            AND S.course_id = T.course_id
    );
```

可使用 `NOT EXISTS` 判断子查询结果是否不包含元组，也可用 `NOT EXISTS (B EXCEPT A)` 判断关系 A 是否完全包含 B。

##### 2.3.8.4 重复元组存在性测试

`UNIQUE` / `NOT UNIQUE` 判断参数集合是否存在重复元组：

```sql
SELECT T.course_id
FROM course AS T
WHERE UNIQUE (
    SELECT R.course_id
    FROM section AS R
    WHERE T.course_id = R.course_id AND R.year = 2009
);
```

**空值处理**：在逻辑表达式中，只要涉及空值，`=` 就返回 `false`。因此若 `UNIQUE` 后的集合中出现 `(A, null)` 和 `(A, null)`，也不会认为它们相等。

##### 2.3.8.5 `FROM` 中的子查询

由于 `SELECT-FROM-WHERE` 的结果是一个关系，因此可插入到其他 `SELECT-FROM-WHERE` 中可出现关系的任意位置。

关于 `FROM` 中的子查询，需补充 `LATERAL` 关键字：它允许 `FROM` 中的子查询访问其**左侧**表的属性。默认 `FROM` 中各表相互独立：

```sql
-- 错误：子查询不认识左侧的 accounts 表
SELECT *
FROM accounts,
     (SELECT * FROM purchases WHERE purchases.account_id = accounts.id) p;
```

使用 `LATERAL` 后：

1. 先处理 `LATERAL` 左侧的表
2. 逐行取出左侧表作为上下文，执行子查询

注意这是消耗性能的操作，例如 `FROM R1, R2, LATERAL (SELECT ...)`，子查询会执行 `|R1| × |R2|` 次。

##### 2.3.8.6 子查询重命名

```sql
(SELECT ... FROM ... WHERE ...) AS new_name (A1, A2, ...);
```

也可省略属性列表：

```sql
(SELECT ... FROM ... WHERE ...) AS new_name;
```

##### 2.3.8.7 `WITH` 子句

SQL:1999 引入 `WITH` 子句，用于定义临时关系：

```sql
WITH max_budget(value) AS (
    SELECT max(budget)
    FROM department
)
SELECT budget
FROM department, max_budget
WHERE department.budget = max_budget.value;
```

##### 2.3.8.8 标量子查询

若子查询返回的元组只有一条且仅有一个元素，则可出现在任何标量可出现的地方：

```sql
SELECT dept_name, (
    SELECT count(*)
    FROM instructor
    WHERE department.dept_name = instructor.dept_name
)
FROM department;
```

### 2.3.9 数据库修改

#### 2.3.9.1 删除

```sql
DELETE FROM r
WHERE Pred;
```

`WHERE` 后可跟子查询，且该子查询可查询被删除的关系本身：

```sql
DELETE FROM instructor
WHERE salary < (
    SELECT avg(salary)
    FROM instructor
);
```

SQL 保证先对所有元组进行测试，再执行删除。

#### 2.3.9.2 插入

##### 插入单条元组

值的顺序须严格符合关系中属性的顺序：

```sql
INSERT INTO course VALUES ('CS-437', 'Database System', 'Comp.Sci', 4);
```

或指定属性顺序：

```sql
INSERT INTO course (title, course_id, credits, dept_name)
VALUES ('Database Systems', 'CS-437', 4, 'Comp.Sci');
```

##### 插入多条元组

`INSERT INTO` 后可跟 `SELECT` 子查询，插入子查询的所有结果：

```sql
INSERT INTO instructor
    SELECT ID, name, dept_name, 18000
    FROM student
    WHERE dept_name = 'Music' AND tot_cred > 144;
```

> 若插入数据中某些属性值未给出，则默认填入空值。

#### 2.3.9.3 更新

```sql
UPDATE R
SET A = xxx
WHERE ...;
```

`WHERE` 后可接 `SELECT` 子句，甚至可引用被更新的关系本身。SQL 保证在更新前检查所有元组。

SQL 提供 `CASE` 结构，支持在 `UPDATE` 中分多种情况更新值：

```sql
UPDATE instructor
SET salary = CASE
    WHEN salary <= 100000 THEN salary * 1.05
    ELSE salary * 1.03
END;
```

`CASE` 通用结构：

```sql
CASE
    WHEN pred1 THEN result1
    WHEN pred2 THEN result2
    ...
    WHEN predn THEN resultn
    ELSE result0
END
```

`CASE` 可出现在任意需要值的地方，也可在 `WHERE` 中按条件筛选：

```sql
SELECT *
FROM city
WHERE CASE
    WHEN city.CountryCode = 'NLD' THEN city.Population > 100000
    WHEN city.CountryCode = 'CHL' THEN city.Population > 200000
    ELSE false
END;
```

更复杂的例子——使用子查询更新学分：

```sql
UPDATE student AS S
SET tot_cred = (
    SELECT sum(credits)
    FROM takes NATURAL JOIN course
    WHERE S.ID = takes.ID
        AND takes.grade != 'F'
        AND takes.grade IS NOT NULL
);
```

若子查询返回空集，则 `sum` 结果为 `NULL` 而非 0，可做如下改进：

```sql
UPDATE student AS S
SET tot_cred = (
    SELECT CASE
        WHEN sum(credits) IS NOT NULL THEN sum(credits)
        ELSE 0
    END
    FROM takes NATURAL JOIN course
    WHERE S.ID = takes.ID
        AND takes.grade != 'F'
        AND takes.grade IS NOT NULL
);
```
