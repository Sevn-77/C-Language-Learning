# 测试报告

## 项目与环境

- **项目：** C11 命令行大学生成绩管理系统。
- **自动化测试：** Python 3 `unittest`，10 个黑盒 CLI 用例，位于 [`tests/test_cli.py`](../tests/test_cli.py)。
- **Windows 编译器：** Visual Studio Build Tools 2022，MSVC 19.44.35229；Windows SDK 10.0.26100。
- **Python：** 3.13.15。
- **隔离方式：** 编译产物和临时成绩文件放在 `tests/.tmp/`，测试结束后清理；不会读取或改写项目根目录中的真实 `students.csv`。

## 最终版本结果

| 检查 | 实际命令/输入 | 预期 | 实际结果 | 状态 |
| --- | --- | --- | --- | --- |
| C 编译和 MSVC 静态分析 | 加载 VS x64 开发环境后运行 `cl /nologo /std:c11 /utf-8 /W4 /analyze /c student-management\main.c`，目标文件写入 `tests/.tmp/` | 编译成功，无静态分析告警 | 退出码 `0`，输出 `main.c`，没有告警 | 通过 |
| Python 测试文件语法 | `%LocalAppData%\Programs\Python\Python313\python.exe -m py_compile tests/test_cli.py` | 文件语法有效 | 退出码 `0` | 通过 |
| CLI 自动化测试 | 加载 VS x64 开发环境后运行 `%LocalAppData%\Programs\Python\Python313\python.exe -m unittest discover -s tests -v` | 编译程序并执行 10 个隔离用例 | `Ran 10 tests in 3.275s`，`OK`，退出码 `0` | **10 通过、0 失败、0 错误、0 跳过** |

Windows 实际执行时使用的开发环境和 Python 可执行文件位于：

```text
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat
%LocalAppData%\Programs\Python\Python313\python.exe
```

完整复现方式见下方命令；Windows 用户应在 x64 Native Tools Command Prompt for VS 2022 中执行。

## 用例明细

| 用例 | 输入或预置数据 | 预期结果 | 最终实际结果 |
| --- | --- | --- | --- |
| 菜单退出 | 选择 `7` | 显示退出信息，返回码为 0 | 通过 |
| 无效及超大菜单值 | `abc`、`8`、`4294967295`，再输入 `7` | 三次均提示无效，仍可正常退出 | 通过 |
| 核心增查改统删 | 增加 `1001/Alice/CS1/90/80/70`；姓名片段 `lic` 查询；高数改为 `100`；统计；删除后查看 | 查到 Alice；个人平均 `80.00`；高数班级均分 `100.00`；删除后名单为空 | 通过 |
| 字段长度边界 | 先输入 50 个 `B`，再输入 49 个 `A`，继续填写 `CS/0/100/80.5` | 拒绝并重新提示超长字段；49 字节字段接受并保存 | 通过 |
| 成绩边界 | 同一成绩依次输入 `-1`、`101`、`nan`、`0`、`100`、`12.5` | 前三项拒绝；`0`、`100`、`12.5` 接受 | 通过 |
| CSV 持久化 | 添加 `2001/Bob/CS2/88/77/66`，退出后重新启动并查看 | CSV 内容保留，第二次启动载入 Bob | 通过 |
| 无效 CSV 行 | 预置有效行、额外尾随字段行、重复学号行、101 分行 | 只载入有效行并报告 3 条无效记录 | 通过 |
| 空数据统计 | 空目录中选择 `5`、`7` | 提示无学生记录，不计算除法 | 通过 |
| 保存失败回滚 | 预置 `6001,Before,CS1,50,60,70`；创建同名临时文件目录阻止写入；依次尝试新增、修改和删除 | 三次提示保存失败；旧 CSV 和内存中的 Before 记录不变 | 通过 |
| 重复学号 | 预置学号 `8001`，再次添加同一学号 | 提示学号重复，CSV 原内容不变 | 通过 |

测试代码在隔离临时目录运行，不会破坏已有成绩数据。未配置覆盖率工具，**覆盖率无法测量**。

## 中间版本缺陷复现

在修复超长文本处理前，实际运行过 10 项测试：9 项通过，`test_exact_field_limit_is_accepted_and_longer_input_is_rejected` 失败。观察到超长文本会取消新增操作。修正读取状态后，最终版本同一用例已通过。中间版本结果不计入最终汇总。

早先将临时可执行文件放到系统 TEMP 时，曾遇到 Windows Device Guard 拦截。测试后改为将生成文件放在项目内的 `tests/.tmp/`（已加入 `.gitignore`）；最终 10 项测试均能实际启动并通过。没有关闭或绕过安全策略。

## 可复现命令

Windows：在 **x64 Native Tools Command Prompt for VS 2022** 中进入项目根目录：

```bat
python -m unittest discover -s tests -v
```

Linux / macOS（需要 GCC）执行：

```sh
python3 -m unittest discover -s tests -v
```
