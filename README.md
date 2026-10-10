# C 语言大学生成绩管理系统

一个面向 C 语言入门学习的命令行项目，用于录入、查询和统计大学生的课程成绩。数据保存在程序当前工作目录中的 CSV 文件里。

## 功能

- 添加学生资料，并检查必填内容和重复学号。
- 查看所有学生及各科成绩。
- 按姓名或姓名片段搜索。
- 修改指定学生的一门课程成绩。
- 删除学生记录。
- 统计每名学生的平均分和各科班级平均分。
- 检查菜单、文本和 0–100 分数输入；拒绝损坏或重复的 CSV 记录。
- 使用临时文件保存，再替换正式 CSV，减少写入中断时破坏旧文件的风险。

课程固定为 C 语言、高等数学和大学英语。程序最多保存 200 名学生。

## 技术栈与依赖

- C11，使用 C 标准库；Windows 下用到系统自带的文件替换 API。
- Windows 推荐 Visual Studio 2022 C++ Build Tools（MSVC）和 Windows SDK；也可使用能编译 C11 的 GCC。
- 自动化 CLI 测试使用 Python 3 标准库 `unittest`，不需要安装第三方 Python 包。

## Windows 编译和运行

安装 Visual Studio 2022 Build Tools 时选择 **使用 C++ 的桌面开发** 工作负载。打开开始菜单中的 **x64 Native Tools Command Prompt for VS 2022**，进入项目根目录后运行：

```bat
cl /nologo /std:c11 /utf-8 /W4 /TC student-management\main.c /Fescore_manager.exe
chcp 65001
score_manager.exe
```

也可以在已加载 MSVC 环境的 VS Code 终端中执行相同命令。若 `cl` 提示不是命令，请从 x64 Native Tools Command Prompt 启动 VS Code，或先使用 VS 的开发者命令环境。

## macOS / Linux 编译和运行

安装 GCC 后，在项目根目录执行：

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 student-management/main.c -o score_manager
./score_manager
```

## 自动化测试

测试会在 `tests/.tmp/` 下编译程序、创建隔离的临时工作目录，并在测试结束时清理数据。不会读取或改写你的真实 `students.csv`。

Windows：在 **x64 Native Tools Command Prompt for VS 2022** 中进入项目根目录：

```bat
python -m unittest discover -s tests -v
```

如果使用 GCC，可在普通终端运行同一命令。macOS / Linux 也可使用 `python3 -m unittest discover -s tests -v`。

当前测试报告和代码审查记录：

- [测试报告](docs/test-report.md)
- [Bug 记录](docs/bug-log.md)

## 持续集成

`.github/workflows/ci.yml` 已配置 GitHub Actions，在 Ubuntu 上使用 GCC 编译并运行 Python CLI 测试。当前工作目录不是 Git 仓库，本次没有触发或验证远程 GitHub Actions。

## 数据和隐私

程序从**当前工作目录**读取 `students.csv`；没有文件时从空名单启动。添加、修改和删除会更新该文件。CSV 字段按逗号分隔，文本不能包含英文逗号。字段缓冲区最多容纳 49 个字节（UTF-8 中文字符通常占多个字节）。

成绩和姓名可能属于个人信息。不要把真实学生数据提交到公开仓库。程序会在当前工作目录中创建 `students.csv`；`.gitignore` 会忽略仓库任意目录中的成绩 CSV、备份和临时保存文件。本次发布前检查未发现成绩 CSV 或数据库文件；测试用例中的姓名和成绩是虚构数据。提交前仍应检查 Git 暂存区，确认没有个人数据。

## 已知限制

- 仅支持三门固定课程；不能添加课程或按多个班级分别管理。
- 最多 200 名学生；没有排序、排名或按学号搜索菜单。
- CSV 不支持字段转义，文本不能包含英文逗号。
- 所有课程都必须有 0–100 分数；不支持缺考、缓考或未录入状态。
- 数据仅保存在本地 CSV；没有用户账号、权限控制、加密、云同步或并发访问支持。
- 本项目适合课程练习和演示，不适合作为正式教务系统。

## 后续改进

1. 支持运行时配置课程、按班级筛选和成绩排名。
2. 为缺考和未录入成绩定义明确的统计规则。
3. 支持标准 CSV 引号转义、备份恢复和数据导入校验。
4. 扩展跨平台与边界条件测试，并持续检查 GitHub Actions 结果。

## 许可证

本项目采用 [MIT License](LICENSE)。版权声明使用 GitHub 用户名 `Sevn-77`。

## 开发协作说明

本项目使用 OpenAI Codex 辅助编写和审查部分 C 代码、自动化测试及文档。学生本人提出了成绩管理项目目标、大学课程范围和 GitHub 展示要求，并参与功能需求确认、测试验收和许可证选择。发布前应由本人阅读代码，并在自己的 Windows/VS Code 环境复现构建和运行；没有亲自完成的工作不应描述为本人独立实现。
