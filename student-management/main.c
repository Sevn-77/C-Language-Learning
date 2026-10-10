#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define MAX_STUDENTS 200
#define FIELD_LENGTH 50
#define SUBJECT_COUNT 3
#define DATA_FILE "students.csv"
#define TEMP_DATA_FILE "students.csv.tmp"

typedef struct {
    char id[FIELD_LENGTH];
    char name[FIELD_LENGTH];
    char className[FIELD_LENGTH];
    double scores[SUBJECT_COUNT];
} Student;

static const char *subjects[SUBJECT_COUNT] = {"C语言", "高等数学", "大学英语"};
static Student students[MAX_STUDENTS];
static int studentCount = 0;

void clearInputLine(void)
{
    int character;
    while ((character = getchar()) != '\n' && character != EOF) {
    }
}

static int readLine(const char *prompt, char *buffer, size_t size)
{
    size_t length;
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else if (!feof(stdin)) {
        int nextCharacter = getchar();
        if (nextCharacter != '\n' && nextCharacter != EOF) {
            clearInputLine();
            printf("输入过长，请控制在 %d 个字节以内。\n", (int)size - 1);
            return -1;
        }
    }
    return 1;
}

static int readRequiredText(const char *prompt, char *buffer, size_t size)
{
    int status;
    while ((status = readLine(prompt, buffer, size)) != 0) {
        if (status < 0) {
            continue;
        }
        if (buffer[0] == '\0') {
            printf("内容不能为空，请重新输入。\n");
            continue;
        }
        if (strchr(buffer, ',') != NULL) {
            printf("内容不能包含英文逗号。\n");
            continue;
        }
        return 1;
    }
    return 0;
}

static int readNumber(const char *prompt, double minimum, double maximum, double *value)
{
    char buffer[FIELD_LENGTH];
    char *end;
    double parsed;
    int status;
    while ((status = readLine(prompt, buffer, sizeof(buffer))) != 0) {
        if (status < 0) {
            continue;
        }
        errno = 0;
        parsed = strtod(buffer, &end);
        while (isspace((unsigned char)*end)) {
            end++;
        }
        if (buffer[0] != '\0' && errno == 0 && *end == '\0' &&
            parsed >= minimum && parsed <= maximum) {
            *value = parsed;
            return 1;
        }
        printf("请输入 %.0f 到 %.0f 之间的有效数字。\n", minimum, maximum);
    }
    return 0;
}

void showMenu(void)
{
    printf("\n===== 学生成绩管理系统 =====\n");
    printf("1. 添加学生信息\n");
    printf("2. 查看所有学生\n");
    printf("3. 按姓名查找学生\n");
    printf("4. 修改学生成绩\n");
    printf("5. 计算平均成绩\n");
    printf("6. 删除学生信息\n");
    printf("7. 退出系统\n");
}

static int readInteger(const char *prompt, int minimum, int maximum)
{
    char buffer[FIELD_LENGTH];
    char *end;
    long choice;
    int status = readLine(prompt, buffer, sizeof(buffer));
    if (status == 0) {
        return -1;
    }
    if (status < 0) {
        return 0;
    }
    errno = 0;
    choice = strtol(buffer, &end, 10);
    while (isspace((unsigned char)*end)) {
        end++;
    }
    if (buffer[0] == '\0' || errno != 0 || *end != '\0') {
        return 0;
    }
    if (choice < minimum || choice > maximum || choice > (long)INT_MAX ||
        choice < (long)INT_MIN) {
        return 0;
    }
    return (int)choice;
}

int readChoice(void)
{
    return readInteger("请输入选项（1-7）：", 1, 7);
}

static double studentAverage(const Student *student)
{
    int i;
    double total = 0.0;
    for (i = 0; i < SUBJECT_COUNT; i++) {
        total += student->scores[i];
    }
    return total / SUBJECT_COUNT;
}

static int findStudentById(const char *id)
{
    int i;
    for (i = 0; i < studentCount; i++) {
        if (strcmp(students[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

static int saveStudents(void)
{
    FILE *file = fopen(TEMP_DATA_FILE, "w");
    int i;
    if (file == NULL) {
        printf("无法创建临时数据文件 %s。\n", TEMP_DATA_FILE);
        return 0;
    }
    for (i = 0; i < studentCount; i++) {
        if (fprintf(file, "%s,%s,%s,%.10g,%.10g,%.10g\n",
                    students[i].id, students[i].name, students[i].className,
                    students[i].scores[0], students[i].scores[1],
                    students[i].scores[2]) < 0) {
            fclose(file);
            remove(TEMP_DATA_FILE);
            printf("写入临时数据文件失败。\n");
            return 0;
        }
    }
    if (fclose(file) != 0) {
        remove(TEMP_DATA_FILE);
        printf("关闭临时数据文件时发生错误。\n");
        return 0;
    }
#ifdef _WIN32
    if (!MoveFileExA(TEMP_DATA_FILE, DATA_FILE,
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(TEMP_DATA_FILE);
        printf("替换数据文件 %s 失败。\n", DATA_FILE);
        return 0;
    }
#else
    if (rename(TEMP_DATA_FILE, DATA_FILE) != 0) {
        remove(TEMP_DATA_FILE);
        printf("替换数据文件 %s 失败。\n", DATA_FILE);
        return 0;
    }
#endif
    return 1;
}

static void loadStudents(void)
{
    FILE *file = fopen(DATA_FILE, "r");
    char line[512];
    int skipped = 0;
    if (file == NULL) {
        return;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        Student student;
        char extra;
        int fields = sscanf(line, "%49[^,],%49[^,],%49[^,],%lf,%lf,%lf %c",
                            student.id, student.name, student.className,
                            &student.scores[0], &student.scores[1],
                            &student.scores[2], &extra);
        if (fields != 6 || student.id[0] == '\0' || student.name[0] == '\0' ||
            student.className[0] == '\0' ||
            studentCount >= MAX_STUDENTS ||
            !isfinite(student.scores[0]) || student.scores[0] < 0 || student.scores[0] > 100 ||
            !isfinite(student.scores[1]) ||
            student.scores[1] < 0 || student.scores[1] > 100 ||
            !isfinite(student.scores[2]) || student.scores[2] < 0 || student.scores[2] > 100 ||
            findStudentById(student.id) >= 0) {
            skipped++;
            continue;
        }
        students[studentCount++] = student;
    }
    fclose(file);
    if (skipped > 0) {
        printf("数据文件中有 %d 条无效记录，已跳过。\n", skipped);
    }
    if (studentCount > 0) {
        printf("已从 %s 载入 %d 位学生。\n", DATA_FILE, studentCount);
    }
}

static void printStudent(const Student *student)
{
    printf("学号：%s | 姓名：%s | 班级：%s\n", student->id,
           student->name, student->className);
    printf("  C语言：%.2f  高等数学：%.2f  大学英语：%.2f  平均分：%.2f\n",
           student->scores[0], student->scores[1], student->scores[2],
           studentAverage(student));
}

static void addStudent(void)
{
    Student student;
    int i;
    if (studentCount >= MAX_STUDENTS) {
        printf("学生名额已满（最多 %d 人）。\n", MAX_STUDENTS);
        return;
    }
    if (!readRequiredText("请输入学号：", student.id, sizeof(student.id)) ||
        !readRequiredText("请输入姓名：", student.name, sizeof(student.name)) ||
        !readRequiredText("请输入班级：", student.className, sizeof(student.className))) {
        printf("输入结束，已取消添加。\n");
        return;
    }
    if (findStudentById(student.id) >= 0) {
        printf("该学号已存在，未添加学生。\n");
        return;
    }
    for (i = 0; i < SUBJECT_COUNT; i++) {
        char prompt[100];
        snprintf(prompt, sizeof(prompt), "请输入%s成绩（0-100）：", subjects[i]);
        if (!readNumber(prompt, 0.0, 100.0, &student.scores[i])) {
            printf("输入结束，已取消添加。\n");
            return;
        }
    }
    students[studentCount++] = student;
    if (saveStudents()) {
        printf("学生信息已添加并保存。\n");
    } else {
        studentCount--;
    }
}

static void listStudents(void)
{
    int i;
    if (studentCount == 0) {
        printf("目前没有学生记录。\n");
        return;
    }
    printf("\n----- 学生名单（%d 人）-----\n", studentCount);
    for (i = 0; i < studentCount; i++) {
        printStudent(&students[i]);
    }
}

static void searchStudent(void)
{
    char name[FIELD_LENGTH];
    int i;
    int found = 0;
    if (!readRequiredText("请输入要查找的姓名或姓名片段：", name, sizeof(name))) {
        printf("输入结束，已取消查找。\n");
        return;
    }
    for (i = 0; i < studentCount; i++) {
        if (strstr(students[i].name, name) != NULL) {
            printStudent(&students[i]);
            found++;
        }
    }
    if (found == 0) {
        printf("没有找到匹配的学生。\n");
    } else {
        printf("共找到 %d 位学生。\n", found);
    }
}

static void updateScore(void)
{
    char id[FIELD_LENGTH];
    double score;
    int index;
    int subjectChoice;
    double oldScore;
    if (!readRequiredText("请输入要修改成绩的学生学号：", id, sizeof(id))) {
        printf("输入结束，已取消修改。\n");
        return;
    }
    index = findStudentById(id);
    if (index < 0) {
        printf("没有找到该学号的学生。\n");
        return;
    }
    printf("请选择科目：1. C语言  2. 高等数学  3. 大学英语\n");
    subjectChoice = readInteger("请输入科目选项（1-3）：", 1, SUBJECT_COUNT);
    if (subjectChoice < 1 || subjectChoice > SUBJECT_COUNT) {
        printf("科目选项无效，已取消修改。\n");
        return;
    }
    if (!readNumber("请输入新成绩（0-100）：", 0.0, 100.0, &score)) {
        printf("输入结束，已取消修改。\n");
        return;
    }
    oldScore = students[index].scores[subjectChoice - 1];
    students[index].scores[subjectChoice - 1] = score;
    if (saveStudents()) {
        printf("%s 的%s成绩已更新并保存。\n",
               students[index].name, subjects[subjectChoice - 1]);
    } else {
        students[index].scores[subjectChoice - 1] = oldScore;
    }
}

static void calculateAverages(void)
{
    double totals[SUBJECT_COUNT] = {0.0, 0.0, 0.0};
    int i;
    int j;
    if (studentCount == 0) {
        printf("目前没有学生记录，无法计算平均成绩。\n");
        return;
    }
    printf("\n----- 学生成绩统计 -----\n");
    for (i = 0; i < studentCount; i++) {
        printf("%s（%s）平均分：%.2f\n", students[i].name,
               students[i].id, studentAverage(&students[i]));
        for (j = 0; j < SUBJECT_COUNT; j++) {
            totals[j] += students[i].scores[j];
        }
    }
    for (j = 0; j < SUBJECT_COUNT; j++) {
        printf("%s班级平均分：%.2f\n", subjects[j], totals[j] / studentCount);
    }
}

static void deleteStudent(void)
{
    char id[FIELD_LENGTH];
    Student removedStudent;
    int index;
    int i;
    if (!readRequiredText("请输入要删除的学生学号：", id, sizeof(id))) {
        printf("输入结束，已取消删除。\n");
        return;
    }
    index = findStudentById(id);
    if (index < 0) {
        printf("没有找到该学号的学生。\n");
        return;
    }
    removedStudent = students[index];
    for (i = index; i < studentCount - 1; i++) {
        students[i] = students[i + 1];
    }
    studentCount--;
    if (saveStudents()) {
        printf("学生记录已删除并保存。\n");
    } else {
        for (i = studentCount; i > index; i--) {
            students[i] = students[i - 1];
        }
        students[index] = removedStudent;
        studentCount++;
    }
}

int main(void)
{
    int choice;
    loadStudents();
    while (1) {
        showMenu();
        choice = readChoice();
        if (choice == -1) {
            printf("\n检测到输入结束，系统已退出。\n");
            break;
        }
        switch (choice) {
        case 1:
            addStudent();
            break;
        case 2:
            listStudents();
            break;
        case 3:
            searchStudent();
            break;
        case 4:
            updateScore();
            break;
        case 5:
            calculateAverages();
            break;
        case 6:
            deleteStudent();
            break;
        case 7:
            printf("系统已退出。\n");
            return 0;
        default:
            printf("输入无效，请输入 1 到 7 之间的数字。\n");
            break;
        }
    }
    return 0;
}
