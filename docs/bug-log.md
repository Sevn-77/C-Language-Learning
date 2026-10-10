# Bug 与风险记录

本文区分实际执行复现和代码检查发现。下列代码修改均已通过最终的 10 项 CLI 自动化回归。

## 已复现并修复

### 中：超长文本会取消当前操作

- **位置：** `student-management/main.c` 的 `readLine`、`readRequiredText` 和 `readNumber`。
- **复现：** 在新增学生时输入一个超过 49 字节的姓名，再输入正常姓名和其余资料。
- **预期：** 提示当前字段过长，然后重新读取该字段。
- **原实际结果：** 超长输入与 EOF 共用返回值，当前新增被取消，后续输入被菜单读取。
- **修复：** 用不同状态区分 EOF、有效输入和超长输入；丢弃过长行后重试。也允许刚好 49 字节的文本。
- **验证：** 最终版本 `test_exact_field_limit_is_accepted_and_longer_input_is_rejected` 通过。

## 代码检查发现并已修复，回归测试通过

### 中：超大菜单数值可能窄化后被误认为 EOF

- **位置：** `readInteger`。
- **发现依据：** 原实现直接把 `strtol` 的 `long` 结果转成 `int`，菜单又用 `-1` 表示 EOF；超出 `int` 范围的值可能与 EOF 哨兵混淆。
- **修复：** 转换前验证调用范围和 `int` 表示范围，越界值作为无效输入处理。
- **验证：** `test_invalid_and_oversized_menu_inputs_do_not_exit` 使用 `abc`、`8`、`4294967295` 验证菜单仍可继续并退出；最终版本通过。

### 中：CSV 直接截断保存，失败时内存状态也不回滚

- **位置：** `saveStudents`、`addStudent`、`updateScore`、`deleteStudent`。
- **发现依据：** 原实现先用写模式打开正式 CSV；新增、修改、删除也会先改变内存再保存。
- **修复：** 写入 `students.csv.tmp` 并成功关闭后再原子替换正式文件；保存失败时恢复内存记录。
- **验证：** `test_failed_save_keeps_memory_and_original_csv_unchanged` 用临时目录阻止临时文件创建，检查原 CSV 与内存显示保持不变；最终版本通过。
- **限制：** 未模拟断电、磁盘故障或硬件写缓存异常。

### 低：CSV 行尾多余数据可能被忽略

- **位置：** `loadStudents`。
- **发现依据：** 原格式串读取完六个字段后没有检查额外数据。
- **修复：** 对记录尾部执行校验，含多余非空字段的行会被跳过。
- **验证：** `test_malformed_trailing_data_and_duplicate_csv_ids_are_skipped` 覆盖多余字段、重复学号和越界成绩；最终版本通过。

## 剩余风险

- CSV 格式仍不支持逗号转义，字段中不能包含英文逗号。
- 固定临时文件名 `students.csv.tmp` 不适合多个程序实例并发写同一数据目录；程序不提供并发访问支持。
- 自动化测试没有覆盖断电、系统崩溃或真实磁盘错误等硬件故障场景。
- 通过当前检查不等于不存在其他 Bug；后续修改仍应重新编译并运行回归。
