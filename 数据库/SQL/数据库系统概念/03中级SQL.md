# CH03. 中级SQL

## 一、连接表达式

### 1.1 连接条件

在[02初级SQL](./02初级SQL.md)中介绍了自然连接以及 `JOIN ... USING` 子句。

SQL 支持另一种形式的连接，允许指定任意的连接条件。

**`ON` 条件**允许在参与连接的关系上设置通用的谓词，出现在连接表达式的末尾：

```sql
SELECT *
FROM student JOIN takes ON student.ID = takes.ID;
```

上面这条语句与 `NATURAL JOIN` 效果几乎一样，但要注意这不是自然连接，只是恰好 `ON` 给出的条件与自然连接相同。一个明显区别是：使用自然连接时，结果关系中相同属性只保留一份；而使用 `ON` 时，`SELECT *` 会包含两个关系的所有属性。

### 1.2 外连接

**外连接**（outer join）与自然连接类似，但会保留包含空值的元组。

外连接有三种模式：

| 类型 | 说明 |
|------|------|
| **左外连接**（left outer join） | 保留左侧关系中的所有元组 |
| **右外连接**（right outer join） | 保留右侧关系中的所有元组 |
| **全外连接**（full outer join） | 保留两个关系中的所有元组 |

左外连接的计算过程：

1. 首先计算自然连接
2. 对于左侧关系中与右侧任何元组都不匹配的元组 `t`，向结果中加入一个元组 `r`：
   - `r` 从左侧关系获得的属性赋值为 `t` 的值
   - `r` 的其他属性（来自右侧关系）赋值为空

右外连接和全外连接是对称的。

#### `ON` 与 `WHERE` 的区别

```sql
-- 查询1：内连接
SELECT * FROM student JOIN takes ON student.ID = takes.ID;

-- 查询2：笛卡尔积后过滤
SELECT * FROM student, takes WHERE student.ID = takes.ID;
```

上面两个查询结果相同。但 `ON` 和 `WHERE` 的语义不同：

- **`ON`** 指定的是**连接条件**，决定两个关系是否匹配
- **`WHERE`** 指定的是**过滤条件**，作用于连接后的结果

下面的例子中两个查询结果不同：

```sql
-- 查询1：左外连接，匹配条件为 ID 相等
SELECT * FROM student LEFT OUTER JOIN takes ON student.ID = takes.ID;

-- 查询2：先笛卡尔积，再过滤
SELECT * FROM student LEFT OUTER JOIN takes ON TRUE
WHERE student.ID = takes.ID;
```

- 查询1：保留没有选课的学生元组，对应 *takes* 属性置为 `null`
- 查询2：匹配条件为 `TRUE`（等价于笛卡尔积），`WHERE` 过滤掉 ID 不相等或为 `null` 的元组，因此没有选课的学生不会被保留

### 1.3 连接类型和条件

SQL 将常规连接称为**内连接**（inner join），关键词为 `INNER JOIN`（可省略 `INNER`，直接写 `JOIN`）。外连接关键词为 `OUTER JOIN`。

连接条件的三种指定方式：

| 方式 | 说明 |
|------|------|
| `NATURAL JOIN` | 自然连接，自动匹配所有同名属性 |
| `JOIN ... ON <条件>` | 指定任意连接条件 |
| `JOIN ... USING (A1, A2, ...)` | 在指定属性上进行自然连接 |

## 二、视图

SQL 允许通过查询定义"虚关系"。它在概念上包含查询的结果，但不会预先计算存储，只有在使用时才执行查询。

这种不是逻辑模型的一部分、但作为虚关系对用户可见的关系称为**视图**（view）。

### 2.1 定义视图

```sql
CREATE VIEW v AS <query expression>;
```

若要指定视图的属性名：

```sql
CREATE VIEW view_name (attr_name1, attr_name2, ...) AS <query expression>;
```

### 2.2 使用视图

视图可以在任何能使用关系的地方使用。但在更新语句中使用视图有较大限制（见 2.4）。

### 2.3 物化视图

有些数据库允许存储视图关系，并保证当定义视图的实际关系改变时视图也同步修改。这种视图称为**物化视图**（materialized view）。

保持物化视图为最新状态的过程称为**物化视图维护**（materialized view maintenance），有三种方式：

| 方式 | 说明 |
|------|------|
| 立即维护 | 定义视图的关系变化时立即更新 |
| 延迟维护 | 仅在访问视图时执行维护 |
| 周期维护 | 周期性进行维护（可能导致访问过期视图） |

### 2.4 视图更新

对视图进行更新、插入和删除可能带来严重问题。主要困难在于：视图表达的数据库修改必须被翻译为对实际关系的修改。

假设有 *instructor(ID, name, dept_name, salary)*，并定义了视图 *faculty_view(ID, name, dept_name)*：

```sql
INSERT INTO faculty_view VALUES ('30765', 'Green', 'Music');
```

该插入需要写入 *instructor*，但未提供 *salary*。有两种处理方式：

- 直接拒绝插入
- 将 *salary* 置为 `null`，即插入 `('30765', 'Green', 'Music', null)`

即使允许置空插入，仍可能存在问题：

```sql
CREATE VIEW instructor_info AS
SELECT ID, name, building
FROM instructor, department
WHERE instructor.dept_name = department.dept_name;

INSERT INTO instructor_info VALUES ('69987', 'White', 'Taylor');
```

该视图涉及两个关系。若允许置空插入，会向 *instructor* 插入 `('69987', 'White', null, null)`，向 *department* 插入 `(null, 'Taylor', null)`。但这两个插入并未实际更新视图（视图查询不会包含它们），因此插入对视图无效。

一般来说，SQL 视图满足以下条件才是可更新的：

- `FROM` 子句只有一个数据库关系
- `SELECT` 子句只包含属性名，不含表达式、聚集或 `DISTINCT`
- 任何未出现在 `SELECT` 中的属性必须可以取空值（无 `NOT NULL` 约束，也不构成主码）
- 查询中不含 `GROUP BY` 或 `HAVING`

即使满足上述条件，仍存在问题：

```sql
CREATE VIEW history_instructors AS
SELECT *
FROM instructor
WHERE dept_name = 'History';
```

向该视图插入 `('25566', 'Brown', 'Biology', 100000)` 是允许的，但该元组不满足 `WHERE` 条件，对视图不可见。

SQL 支持在视图定义末尾添加 `WITH CHECK OPTION` 来增加限制：

```sql
CREATE VIEW v AS
SELECT ...
FROM ...
WHERE ...
WITH CHECK OPTION;
```

该选项会拒绝不满足 `WHERE` 条件的插入。

## 三、事务

**事务**（transaction）由查询和/或更新语句的序列组成。

SQL 规定：**每执行一条 SQL 语句，就隐式开启一个事务**。

以下语句之一会结束一个事务：

- **`COMMIT WORK`**：提交当前事务，之后新事务自动开始
- **`ROLLBACK WORK`**：回滚当前事务，数据库回到执行该事务第一条语句之前的状态

> 关键词 `WORK` 是可选的。

数据库系统保证在发生 SQL 语句错误、断电或系统崩溃时，若未完成 `COMMIT`，则影响被回滚。

很多 SQL 实现中，默认每条 SQL 语句自成一个事务并立即提交。若要让一个事务执行多条 SQL 语句，需关闭自动提交。

SQL:1999 提供 `BEGIN ATOMIC ... END` 关键字，允许将多条 SQL 语句构成一个单独的事务。

## 四、完整性约束

完整性约束的主要目的是保证授权用户对数据库的修改不会破坏数据的一致性。

完整性约束可以是属于数据库的任意谓词，但若不加限定，用户可能给出检测代价高昂的谓词。因此大多数数据库只允许用户指定只需极小开销就能检测的完整性约束。

完整性约束可在 `CREATE TABLE` 中添加，也可通过 `ALTER TABLE xxx ADD CONSTRAINT` 添加。

### 4.1 单关系约束

除主码约束外，SQL 还支持：

| 约束 | 说明 |
|------|------|
| `NOT NULL` | 属性不允许为空 |
| `UNIQUE` | 属性取值必须唯一 |
| `CHECK(谓词)` | 每个元组必须满足指定谓词 |

### 4.2 `NOT NULL` 约束

```sql
name VARCHAR(20) NOT NULL;
```

主码不允许为空，因此不必显式声明 `NOT NULL`。

### 4.3 `UNIQUE` 约束

**单属性约束**：

```sql
-- 列级
CREATE TABLE t (
    name CHAR(20) UNIQUE,
    ...
);

-- 表级（可自定义约束名）
CREATE TABLE users (
    id INT PRIMARY KEY,
    email VARCHAR(100),
    username VARCHAR(50),
    CONSTRAINT uq_email UNIQUE (email)
);
```

**多属性约束**：

```sql
CREATE TABLE course_selection (
    student_id INT,
    course_id INT,
    semester VARCHAR(10),
    UNIQUE (student_id, course_id, semester)   -- 同一学生同一学期不能重复选课
);
```

### 4.4 `CHECK` 子句

`CHECK` 可指定约束谓词，关系中每个元组都必须满足。

**列级约束**：

```sql
CREATE TABLE students (
    id INT PRIMARY KEY,
    age INT CHECK (age >= 18),
    gender CHAR(1) CHECK (gender IN ('M', 'F'))
);
```

**表级约束（可跨列）**：

```sql
CREATE TABLE orders (
    order_id INT PRIMARY KEY,
    quantity INT,
    price DECIMAL(10,2),
    discount DECIMAL(10,2),
    CONSTRAINT chk_total CHECK (quantity * price * (1 - discount) > 0)
);
```

### 4.5 参照完整性

**参照完整性**（referential integrity）：一个关系中给定属性集上的取值，必须在另一个关系的特定属性集中存在。

**外码**（foreign key）的形式化定义：令关系 r1 和 r2 的属性集分别为 R1 和 R2，主码分别为 K1 和 K2。若对于 r2 中的任意元组 t2，均存在 r1 中的元组 t1 使得 `t1.K1 = t2.α`，则称 R2 的子集 α 为参照关系 r1 中 K1 的外码。

这种要求称为**参照完整性约束**（referential-integrity constraints）或**子集依赖**（subset dependency），因为 r2 在 α 上的取值集合必须是 r1 的 K1 上的取值集合的子集。

> 与外码约束不同，参照完整性约束不要求 K2 是 r2 的主码。

SQL 的 `REFERENCES` 子句要求被参照的属性列表**必须声明为参照关系的候选码**（`PRIMARY KEY` 或 `UNIQUE`）。但更普遍的参照完整性约束中，被参照的属性不必是候选码。

**列级写法**：

```sql
CREATE TABLE orders (
    order_id INT PRIMARY KEY,
    customer_id INT REFERENCES customers(customer_id),
    order_date DATE
);
```

**表级写法**（支持更多选项）：

```sql
CREATE TABLE orders (
    order_id INT PRIMARY KEY,
    customer_id INT,
    order_date DATE,
    CONSTRAINT fk_orders_customers
        FOREIGN KEY (customer_id) REFERENCES customers(customer_id)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
);
```

违约处理选项：

| 子句 | 说明 |
|------|------|
| `ON DELETE CASCADE` | 删除被参照元组时，级联删除参照元组 |
| `ON UPDATE CASCADE` | 更新被参照主码时，级联更新参照元组 |
| `ON DELETE SET NULL` | 删除被参照元组时，将参照属性置为 `null` |
| `ON UPDATE SET DEFAULT` | 更新被参照主码时，将参照属性置为默认值 |

### 4.6 事务中对完整性约束的违反

事务中可能出现完整性约束暂时违反、后续步骤再消除的情况。

例如关系 *person(name, spouse)*，*spouse* 是引用 *person.name* 的外键。若插入夫妻信息 `(John, Mary)` 和 `(Mary, John)`，无论先插入哪一条，都会因 *spouse* 不存在而违反外键约束；但两条都插入后约束即满足。

SQL 标准提供了 `INITIALLY DEFERRED` 子句，使约束检查推迟到事务结束时进行：

```sql
CREATE TABLE users (
    user_id INT PRIMARY KEY,
    name VARCHAR(100),
    last_order_id INT,
    CONSTRAINT fk_users_last_order
        FOREIGN KEY (last_order_id) REFERENCES orders(order_id)
        DEFERRABLE INITIALLY DEFERRED
);
```

约束属性说明：

| 属性 | 含义 |
|------|------|
| `DEFERRABLE` | 该约束可延迟检查（默认 `NOT DEFERRABLE`） |
| `INITIALLY DEFERRED` | 初始检查时机为延迟（事务结束时检查，默认 `INITIALLY IMMEDIATE`） |

若约束为 `DEFERRABLE` 且 `INITIALLY IMMEDIATE`，可在事务中手动推迟：

```sql
SET CONSTRAINTS constraint-list DEFERRED;
```

### 4.7 复杂 CHECK 条件与断言

SQL 标准支持在 `CHECK` 子句中使用子查询：

```sql
CHECK (time_slot_id IN (SELECT time_slot_id FROM time_slot))
```

该 `CHECK` 涉及 *section* 和 *time_slot*，因此在插入 *section* 和修改 *time_slot* 时都需要检查。

当需要建立参照性约束，但被参照属性不是候选码时，若数据库支持 `CHECK` 子查询，可用来替代 `REFERENCES`。

#### 4.7.1 断言

**断言**（assertion）是一个谓词，表达数据库应始终满足的条件。域约束（`NOT NULL`）和参照完整性约束是断言的特殊形式。

```sql
CREATE ASSERTION <assertion-name> CHECK <Predicate>;
```

使用断言需非常小心，过于复杂的断言会带来极大的开销。

> 目前还没有广泛使用的数据库支持在 `CHECK` 中添加子查询或 `CREATE ASSERTION` 语法。

## 五、SQL 的数据类型与模式

### 5.1 日期和时间类型

SQL 标准支持以下日期/时间类型：

| 类型 | 说明 | 示例 |
|------|------|------|
| `DATE` | 日历日期（年、月、日） | `'2001-04-25'` |
| `TIME` | 一天中的时间（时、分、秒），可加 `WITH TIMEZONE` | `'09:30:00'` |
| `TIMESTAMP` | `DATE` + `TIME`，可加 `WITH TIMEZONE` | `'2001-04-25 10:29:01.45'` |

时间精度可用 `TIME(p)` 或 `TIMESTAMP(p)` 指定秒的小数位。

相关函数：

- `CAST(e AS T)`：将字符串 `e` 转换为日期/时间类型 `T`
- `EXTRACT(field FROM d)`：从日期/时间中提取 `YEAR`、`MONTH`、`DAY`、`HOUR`、`MINUTE`、`SECOND`、`TIMEZONE_HOUR`、`TIMEZONE_MINUTE`

当前时间函数：

| 函数 | 返回值 |
|------|--------|
| `CURRENT_DATE` | 当前日期 |
| `CURRENT_TIME` | 当前时间（带时区） |
| `LOCALTIME` | 当前时间（不带时区） |
| `CURRENT_TIMESTAMP` | 当前时间戳（带时区） |
| `LOCALTIMESTAMP` | 当前时间戳（不带时区） |

SQL 还支持 `INTERVAL` 类型，表示一段时间，可通过时间类型相减获得。以上类型均支持比较运算。

### 5.2 默认值

```sql
CREATE TABLE student (
    ID VARCHAR(5),
    name VARCHAR(20) NOT NULL,
    tot_cred NUMERIC(3,0) DEFAULT 0,
    PRIMARY KEY (ID)
);
```

### 5.3 索引

默认情况下，表中元组无序存放。在属性上创建**索引**（index）可加速查找。

索引可理解为对属性值建立的一份排序表（常用 B+ 树实现）。SQL 未定义创建索引的标准语法，但多数数据库支持：

```sql
CREATE INDEX studentID_index ON student(ID);
```

### 5.4 大对象

SQL 提供两种大对象类型（LOB：Large Object）：

| 类型 | 说明 |
|------|------|
| `CLOB` | 字符大对象 |
| `BLOB` | 二进制大对象 |

```sql
book_review CLOB(10KB)
image BLOB(10MB)
movie BLOB(2GB)
```

更高效的做法是用 SQL 检索大对象的"句柄"，然后在宿主语言中流式处理。

### 5.5 用户定义的数据类型

SQL 支持两种用户自定义类型：**独特类型**（distinct type）和**结构化数据类型**（structured data type），这里仅介绍前者。

```sql
CREATE TYPE Dollars AS NUMERIC(12, 2) FINAL;
CREATE TYPE Pounds AS NUMERIC(12, 2) FINAL;
```

> `FINAL` 是 SQL:1999 标准要求的关键词，一些实现允许省略。

创建类型后可用于定义表的属性。强类型检查可避免不同类型间的误赋值（如将 `Pounds` 赋值给 `Dollars` 是不允许的）。

SQL 支持 `DROP TYPE` 和 `ALTER TYPE`。

早期 SQL 还提出过**域**（domain），它是基本类型加上完整性约束：

```sql
CREATE DOMAIN Dollars AS NUMERIC(12, 2) NOT NULL;
```

域与类型的区别：

| 特性 | 域 | 类型 |
|------|----|------|
| 约束声明 | 可声明 `NOT NULL`、默认值等 | 不可 |
| 类型检查 | 不强（底层类型一致可互相赋值） | 强 |

可在域上添加 `CHECK` 子句：

```sql
CREATE DOMAIN YearlySalary NUMERIC(8,2)
CONSTRAINT salary_value_test CHECK (VALUE >= 29000.00);
```

> 并非所有数据库都支持 `CREATE TYPE` 和 `CREATE DOMAIN`。

### 5.6 `CREATE TABLE` 的扩展

```sql
-- 创建与另一表同模式的表
CREATE TABLE temp_instructor LIKE instructor;

-- 将查询结果存储为新表（SQL:2003）
CREATE TABLE t1 AS (
    SELECT *
    FROM instructor
    WHERE dept_name = 'Music'
) WITH DATA;
```

SQL:2003 规定省略 `WITH DATA` 则只建表不载入数据，但很多实现在省略时默认也会载入数据。这种方法创建的表会被持久化保存。

### 5.7 模式、目录与环境

现代数据库提供三层结构的关系命名规则：

1. **目录**（catalog，有些数据库称为"数据库"）
2. **模式**（schema）
3. 关系和视图等 SQL 对象

通常的 SQL 语句（包括 DDL 和 DML）都在一个模式的环境中运行。使用 `CREATE SCHEMA` 和 `DROP SCHEMA` 来创建和删除模式。

## 六、授权

对数据的授权包括：

- 授权**读取**数据（`SELECT`）
- 授权**插入**数据（`INSERT`）
- 授权**更新**数据（`UPDATE`）
- 授权**删除**数据（`DELETE`）

每种授权称为一个**权限**（privilege）。用户还可被授予在数据库模式上的权限（创建、修改或删除关系）。拥有权限的用户可转授权限给其他用户。

### 6.1 权限的授予与收回

**授予权限**：

```sql
GRANT <权限列表>
ON <关系名/视图名>
TO <用户/角色列表>;
```

示例：

```sql
GRANT SELECT ON department TO Amit, Satoshi;
GRANT UPDATE (budget) ON department TO Amit, Satoshi;
```

`PUBLIC` 表示系统中的所有用户。

**收回权限**：

```sql
REVOKE <权限列表>
ON <关系名/视图名>
FROM <用户/角色列表>;
```

### 6.2 角色

**角色**（role）是批量管理用户权限的机制。创建角色后，可像给用户授权一样给角色授权，再将角色授予用户或其他角色。

```sql
CREATE ROLE instructor;

GRANT dean TO Amit;
CREATE ROLE dean;
GRANT instructor TO dean;
GRANT dean TO Satoshi;
```

一个用户的权限包括：
- 所有直接授予该用户的权限
- 所有授予该用户的角色链上的权限

### 6.3 视图权限

**视图无法授予用户超过其在该关系上已有权限的权限**。例如用户在视图上执行 `SELECT`，则他必须在视图背后的关系上具有 `SELECT` 权限。

SQL 支持创建函数和过程，并可授予用户 **`EXECUTE`** 权限。默认情况下函数/过程执行时使用其创建者的权限。从 SQL:2003 开始，若函数定义包含 `SQL SECURITY INVOKER` 子句，则在调用者的权限下执行。

### 6.4 模式授权

只有模式的拥有者才能执行对模式的任何修改（创建/删除关系、增删属性、增删索引）。

SQL 提供 **`REFERENCES`** 权限，允许用户在创建关系时声明外码：

```sql
GRANT REFERENCES (dept_name) ON department TO Mariano;
```

该权限允许用户 Mariano 创建参照 `department.dept_name` 属性的关系。设计 `REFERENCES` 权限的原因是：创建参照约束后，被参照关系不能随意删除被引用的元组。

### 6.5 授权的转移

默认用户/角色无法将授予自己的权限转授给他人。在授权时添加 `WITH GRANT OPTION` 即可允许转授：

```sql
GRANT SELECT ON department TO Amit WITH GRANT OPTION;
```

回收 grant option：

```sql
REVOKE GRANT OPTION FOR SELECT ON department FROM Amit;
```

转授关系会形成一张**授权图**（authorization graph）：

![授权图](./authgraph.png)

用户/角色拥有权限的充要条件：**存在从授权图根节点到该用户顶点的路径**。

### 6.6 权限的收回

权限收回默认是级联的（会收回所有依赖该权限的转授）。可通过 `RESTRICT` 阻止级联收回：

```sql
REVOKE SELECT ON department FROM Amit, Satoshi RESTRICT;
```

但 `RESTRICT` 并非在图中添加授权线，而是检测若收回导致级联收回则直接拒绝操作。

一种更合理的做法是**以角色而非用户来完成授权**。可通过以下语句指定当前会话的角色：

```sql
SET ROLE role_name;
```

在授权语句末尾添加 `GRANTED BY CURRENT_ROLE`，则授权者角色而非当前用户，从而避免用户离职导致权限链断裂：

```sql
GRANT SELECT ON department TO ... GRANTED BY CURRENT_ROLE;
```
