# 学生成绩管理系统（入门版）

这是一个用 C 语言编写的控制台菜单程序。目前实现了菜单显示、选项输入、非法输入提示和退出功能。学生信息与成绩相关功能会暂时显示“功能尚未实现”。

## 编译和运行

请先安装 GCC（例如 MinGW-w64）或其他 C 编译器，然后在仓库根目录执行：

```sh
gcc -std=c11 -Wall -Wextra -o student-management-program student-management/main.c
./student-management-program
```

如果使用 Windows 命令提示符，编译后运行：

```text
gcc -std=c11 -Wall -Wextra -o student-management-program.exe student-management/main.c
student-management-program.exe
```

## 菜单选项

1. 添加学生信息
2. 查看所有学生
3. 按姓名查找学生
4. 修改学生成绩
5. 计算平均成绩
6. 退出系统

选择 1 到 5 会显示“功能尚未实现”；选择 6 会结束程序。输入非数字或超出 1 到 6 的数字时，程序会提示重新输入。输入结束（例如终端中按 Ctrl+D；Windows 控制台通常按 Ctrl+Z 后回车）时，程序也会退出。

## 函数说明

- `clearInputLine`：读掉当前输入行剩余的字符，避免它们被当成下一次菜单输入。
- `showMenu`：打印主菜单。
- `readChoice`：读取一个整数选项；非数字时返回 0，输入结束时返回 -1。
- `showNotImplemented`：显示当前功能尚未实现的提示。
- `main`：程序入口。它循环显示菜单、读取选项并用 `switch` 分发处理；用户选择 6 或输入结束时退出。

## 执行流程

程序从 `main` 开始，先调用 `showMenu` 展示菜单，再调用 `readChoice` 读取用户输入。随后 `switch` 根据选项显示尚未实现提示、报告非法选项，或结束程序。除退出外，程序会回到循环开头继续显示菜单。

## 本阶段涉及的知识

这里用到了变量、函数、循环、条件判断、`switch` 分支和标准输入输出。这些都属于你已经学过的内容。后续实现学生信息保存时，再逐步引入数组和字符串；这个版本没有使用结构体或动态内存。
