#include <stdio.h>

/*
 * 清理本行剩余输入，避免错误内容影响下一次菜单选择。
 */
void clearInputLine(void)
{
    int character;

    while ((character = getchar()) != '\n' && character != EOF) {
        /* 逐个读取并丢弃本行剩余字符 */
    }
}

/*
 * 显示系统菜单。
 */
void showMenu(void)
{
    printf("\n===== 学生成绩管理系统 =====\n");
    printf("1. 添加学生信息\n");
    printf("2. 查看所有学生\n");
    printf("3. 按姓名查找学生\n");
    printf("4. 修改学生成绩\n");
    printf("5. 计算平均成绩\n");
    printf("6. 退出系统\n");
}

/*
 * 读取菜单选项。
 * 返回 -1 表示输入结束，返回 0 表示输入不是有效整数。
 */
int readChoice(void)
{
    int choice;
    int result = scanf("%d", &choice);

    if (result == EOF) {
        return -1;
    }

    if (result != 1) {
        clearInputLine();
        return 0;
    }

    clearInputLine();
    return choice;
}

/*
 * 显示功能暂未实现的提示。
 */
void showNotImplemented(void)
{
    printf("功能尚未实现。\n");
}

/*
 * 程序入口：反复显示菜单，直到用户选择退出或输入结束。
 */
int main(void)
{
    int choice;

    while (1) {
        showMenu();
        printf("请输入选项（1-6）：");

        choice = readChoice();

        if (choice == -1) {
            printf("\n检测到输入结束，系统已退出。\n");
            break;
        }

        switch (choice) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            showNotImplemented();
            break;
        case 6:
            printf("系统已退出。\n");
            return 0;
        default:
            printf("输入无效，请输入 1 到 6 之间的数字。\n");
            break;
        }
    }

    return 0;
}
