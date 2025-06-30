#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>
#include <direct.h>  // Windows 目录操作
#include <errno.h>   // 错误处理

#define MAX_N 320000      // 最大物品数量
#define MAX_W 1000000     // 最大背包容量
#define MAX_DP_CAPACITY 100000 // 动态规划的最大容量限制

// 创建输出目录函数
void ensureOutputDirectory() {
    // 检查目录是否存在
    if (_access("output", 0) == -1) {
        // 目录不存在，尝试创建
        if (_mkdir("output") != 0) {
            printf("无法创建输出目录 'output'，错误代码: %d\n", errno);
        }
        else {
            printf("已创建输出目录 'output\\'\n");
        }
    }
}

// 获取带路径的文件名
void getOutputFilePath(char* buffer, size_t size, const char* filename) {
    snprintf(buffer, size, "output\\%s", filename);
}

typedef struct {
    int weight;
    double value;
} Item;

Item items[MAX_N];
int item_count;
int knapsack_capacity;
double max_value;
FILE* result_file = NULL;

// 函数声明
void clearInputBuffer();
bool isValidNumber(const char* str);
bool parseItemInput(const char* input, int* weight, int* value);
void displayItems();
void saveItemsToCSV(const char* filename);
void solveWithDynamicProgramming();
void solveWithBruteForce();
void solveWithGreedy();
void backtrackRecursive(int level, double current_weight, double current_value,
    double* max_val, int* selected, int* best_selected);
void solveWithBacktracking();
void inputKnapsackCapacity();
void inputItemCount();
void inputItems();
void generateRandomItems();
void heapSortDensities(double densities[], int indices[], int n);
void runPerformanceTests();

// 清除输入缓冲区
void clearInputBuffer() {
    while (getchar() != '\n');
}

// 检查字符串是否只包含数字
bool isValidNumber(const char* str) {
    if (str == NULL || *str == '\0') return false;
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}

// 解析逗号分隔的输入
bool parseItemInput(const char* input, int* weight, int* value) {
    char inputCopy[50];
    strncpy(inputCopy, input, sizeof(inputCopy));
    inputCopy[sizeof(inputCopy) - 1] = '\0';

    char* token = strtok(inputCopy, ",");
    if (token == NULL) {
        // 尝试中文逗号
        strncpy(inputCopy, input, sizeof(inputCopy));
        token = strtok(inputCopy, "，");
    }
    if (token == NULL) return false;

    char* endptr;
    *weight = strtol(token, &endptr, 10);
    if (*endptr != '\0' || *weight <= 0) return false;

    token = strtok(NULL, ",");
    if (token == NULL) {
        // 尝试中文逗号
        token = strtok(NULL, "，");
    }
    if (token == NULL) return false;

    *value = strtol(token, &endptr, 10);
    if (*endptr != '\0' || *value <= 0) return false;

    return true;
}

// 显示当前物品信息
void displayItems() {
    printf("\n当前物品信息:\n");
    printf("物品数量: %d\n", item_count);
    printf("背包容量: %d\n", knapsack_capacity);
    printf("物品编号\t重量\t价值\t价值密度\n");

    int displayCount = item_count > 1000 ? 1000 : item_count;
    for (int i = 0; i < displayCount; i++) {
        double density = items[i].value / items[i].weight;
        printf("%d\t\t%d\t%.2f\t%.2f\n", i + 1, items[i].weight, items[i].value, density);
    }
    if (item_count > 1000) {
        printf("... (共%d个物品，只显示前1000个)\n", item_count);
    }
}

// 保存物品信息到文件
void saveItemsToCSV(const char* filename) {
    char filepath[256];
    getOutputFilePath(filepath, sizeof(filepath), filename);

    FILE* fp = fopen(filepath, "w");
    if (fp == NULL) {
        printf("无法创建文件 %s (错误代码: %d)\n", filepath, errno);
        return;
    }

    fprintf(fp, "物品编号,重量,价值\n");
    int saveCount = item_count > 1000 ? 1000 : item_count;
    for (int i = 0; i < saveCount; i++) {
        fprintf(fp, "%d,%d,%.2f\n", i + 1, items[i].weight, items[i].value);
    }
    fclose(fp);
    printf("物品信息已保存到 %s\n", filepath);
}

// 动态规划法求解
void solveWithDynamicProgramming() {
    if (knapsack_capacity > MAX_DP_CAPACITY) {
        printf("\n动态规划法求解结果:\n");
        printf("警告: 背包容量过大(%d > %d)，动态规划法可能消耗过多内存!\n",
            knapsack_capacity, MAX_DP_CAPACITY);
        printf("建议使用其他算法或减小背包容量。\n");
        return;
    }

    clock_t startTime = clock();

    // 分配动态规划表内存
    double** dp = (double**)malloc((item_count + 1) * sizeof(double*));
    for (int i = 0; i <= item_count; i++) {
        dp[i] = (double*)malloc((knapsack_capacity + 1) * sizeof(double));
        memset(dp[i], 0, (knapsack_capacity + 1) * sizeof(double));
    }

    // 填充动态规划表
    for (int i = 1; i <= item_count; i++) {
        for (int w = 1; w <= knapsack_capacity; w++) {
            dp[i][w] = dp[i - 1][w];

            if (items[i - 1].weight <= w) {
                double valueIfSelected = dp[i - 1][w - items[i - 1].weight] + items[i - 1].value;
                if (valueIfSelected > dp[i][w]) {
                    dp[i][w] = valueIfSelected;
                }
            }
        }
    }

    max_value = dp[item_count][knapsack_capacity];

    // 回溯求解选择的物品
    int* selected = (int*)calloc(item_count, sizeof(int));
    double total_weight = 0;
    int w = knapsack_capacity;
    for (int i = item_count; i > 0; i--) {
        if (dp[i][w] != dp[i - 1][w]) {
            selected[i - 1] = 1;
            w -= items[i - 1].weight;
            total_weight += items[i - 1].weight;
        }
    }

    clock_t endTime = clock();
    double time_used = ((double)(endTime - startTime)) / CLOCKS_PER_SEC * 1000;

    // 打印结果
    printf("\n动态规划法求解结果:\n");
    printf("最大价值: %.2f\n", max_value);
    printf("计算时间: %.2f 毫秒\n", time_used);

    // 打印选择的物品信息
    printf("选择的物品编号: ");
    for (int i = 0; i < item_count; i++) {
        if (selected[i]) {
            printf("%d ", i + 1);
        }
    }

    printf("\n\n选择的物品详细信息:\n");
    printf("物品编号\t重量\t价值\n");
    for (int i = 0; i < item_count; i++) {
        if (selected[i]) {
            printf("%d\t\t%d\t%.2f\n", i + 1, items[i].weight, items[i].value);
        }
    }
    printf("\n总重量: %.2f (剩余容量: %.2f)\n", total_weight, knapsack_capacity - total_weight);

    // 保存详细结果
    if (item_count == 1000 && knapsack_capacity == 5000) {
        char filepath[256];
        getOutputFilePath(filepath, sizeof(filepath), "sp_results.csv");

        result_file = fopen(filepath, "w");
        if (result_file) {
            fprintf(result_file, "算法类型,动态规划法\n");
            fprintf(result_file, "物品数量,%d\n", item_count);
            fprintf(result_file, "背包容量,%d\n\n", knapsack_capacity);
            fprintf(result_file, "最大价值,%.2f\n", max_value);
            fprintf(result_file, "计算时间(毫秒),%.2f\n", time_used);

            fprintf(result_file, "\n物品编号,重量,价值,是否选中\n");
            for (int i = 0; i < item_count; i++) {
                fprintf(result_file, "%d,%d,%.2f,%d\n",
                    i + 1, items[i].weight, items[i].value, selected[i]);
            }
            fclose(result_file);
            result_file = NULL;
            printf("\n动态规划法详细结果已保存到 %s\n", filepath);
        }
        else {
            printf("无法创建文件 %s (错误代码: %d)\n", filepath, errno);
        }
    }

    // 释放内存
    for (int i = 0; i <= item_count; i++) {
        free(dp[i]);
    }
    free(dp);
    free(selected);
}

// 蛮力法求解
void solveWithBruteForce() {
    clock_t start = clock();
    double max_val = 0;
    int best_combination = 0;

    // 遍历所有可能的组合 (2^n种可能)
    for (int i = 0; i < (1 << item_count); i++) {
        double total_weight = 0;
        double total_value = 0;

        for (int j = 0; j < item_count; j++) {
            if (i & (1 << j)) {
                total_weight += items[j].weight;
                total_value += items[j].value;
            }
        }

        // 如果当前组合满足重量限制且价值更大，则更新最大值
        if (total_weight <= knapsack_capacity && total_value > max_val) {
            max_val = total_value;
            best_combination = i;
        }
    }

    max_value = max_val;
    clock_t end = clock();
    double time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    // 打印结果
    printf("\n蛮力法求解结果:\n");
    printf("最大价值: %.2f\n", max_value);
    printf("计算时间: %.2f 毫秒\n", time_used);

    // 显示选择的物品详细信息
    printf("\n选择的物品详细信息:\n");
    printf("物品编号\t重量\t价值\n");
    double total_w = 0;
    int* selected = (int*)calloc(item_count, sizeof(int));
    for (int i = 0; i < item_count; i++) {
        if (best_combination & (1 << i)) {
            printf("%d\t\t%d\t%.2f\n", i + 1, items[i].weight, items[i].value);
            total_w += items[i].weight;
            selected[i] = 1;
        }
    }
    printf("\n总重量: %.2f (剩余容量: %.2f)\n", total_w, knapsack_capacity - total_w);

    // 保存详细结果
    if (item_count == 1000 && knapsack_capacity == 5000) {
        char filepath[256];
        getOutputFilePath(filepath, sizeof(filepath), "sp_results.csv");

        result_file = fopen(filepath, "a");  // 追加模式
        if (result_file) {
            fprintf(result_file, "\n\n算法类型,蛮力法\n");
            fprintf(result_file, "物品数量,%d\n", item_count);
            fprintf(result_file, "背包容量,%d\n\n", knapsack_capacity);
            fprintf(result_file, "最大价值,%.2f\n", max_value);
            fprintf(result_file, "计算时间(毫秒),%.2f\n", time_used);

            fprintf(result_file, "\n物品编号,重量,价值,是否选中\n");
            for (int i = 0; i < item_count; i++) {
                fprintf(result_file, "%d,%d,%.2f,%d\n",
                    i + 1, items[i].weight, items[i].value, selected[i]);
            }
            fclose(result_file);
            result_file = NULL;
            printf("\n蛮力法详细结果已追加到 %s\n", filepath);
        }
        else {
            printf("无法追加到文件 %s (错误代码: %d)\n", filepath, errno);
        }
    }

    free(selected);
}

// 堆排序辅助函数
void heapify(double arr[], int idx[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        double temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;

        int temp_idx = idx[i];
        idx[i] = idx[largest];
        idx[largest] = temp_idx;

        heapify(arr, idx, n, largest);
    }
}

// 堆排序
void heapSortDensities(double densities[], int indices[], int n) {
    // 构建最大堆
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(densities, indices, n, i);

    // 提取元素
    for (int i = n - 1; i > 0; i--) {
        // 移动当前根到末尾
        double temp = densities[0];
        densities[0] = densities[i];
        densities[i] = temp;

        int temp_idx = indices[0];
        indices[0] = indices[i];
        indices[i] = temp_idx;

        // 在减少的堆上调用heapify
        heapify(densities, indices, i, 0);
    }
}

// 贪心算法
void solveWithGreedy() {
    if (item_count <= 0 || knapsack_capacity <= 0) {
        printf("无效的物品数量或背包容量!\n");
        return;
    }

    clock_t start = clock();

    // 创建索引数组并计算价值密度
    int* indices = (int*)malloc(item_count * sizeof(int));
    double* densities = (double*)malloc(item_count * sizeof(double));
    if (!indices || !densities) {
        printf("内存分配失败!\n");
        free(indices);
        free(densities);
        return;
    }

    for (int i = 0; i < item_count; i++) {
        indices[i] = i;
        densities[i] = items[i].value / items[i].weight;
    }

    // 使用堆排序按价值密度降序排序
    heapSortDensities(densities, indices, item_count);

    double current_weight = 0;
    max_value = 0;
    int* selected = (int*)calloc(item_count, sizeof(int));
    if (!selected) {
        printf("内存分配失败!\n");
        free(indices);
        free(densities);
        return;
    }

    // 选择物品（从价值密度最高的开始）
    for (int i = item_count - 1; i >= 0; i--) {
        int idx = indices[i];
        if (current_weight + items[idx].weight <= knapsack_capacity) {
            selected[idx] = 1;
            current_weight += items[idx].weight;
            max_value += items[idx].value;
        }
    }

    clock_t end = clock();
    double time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    // 打印结果
    printf("\n贪心算法求解结果:\n");
    printf("最大价值: %.2f\n", max_value);
    printf("计算时间: %.2f 毫秒\n", time_used);

    printf("\n选择的物品详细信息:\n");
    printf("物品编号\t重量\t价值\n");
    for (int i = 0; i < item_count; i++) {
        if (selected[i]) {
            printf("%d\t\t%d\t%.2f\n", i + 1, items[i].weight, items[i].value);
        }
    }
    printf("\n总重量: %.2f (剩余容量: %.2f)\n", current_weight, knapsack_capacity - current_weight);

    // 保存详细结果
    if (item_count == 1000 && knapsack_capacity == 5000) {
        char filepath[256];
        getOutputFilePath(filepath, sizeof(filepath), "sp_results.csv");

        result_file = fopen(filepath, "a");  // 追加模式
        if (result_file) {
            fprintf(result_file, "\n\n算法类型,贪心算法\n");
            fprintf(result_file, "物品数量,%d\n", item_count);
            fprintf(result_file, "背包容量,%d\n\n", knapsack_capacity);
            fprintf(result_file, "最大价值,%.2f\n", max_value);
            fprintf(result_file, "计算时间(毫秒),%.2f\n", time_used);

            fprintf(result_file, "\n物品编号,重量,价值,是否选中\n");
            for (int i = 0; i < item_count; i++) {
                fprintf(result_file, "%d,%d,%.2f,%d\n",
                    i + 1, items[i].weight, items[i].value, selected[i]);
            }
            fclose(result_file);
            result_file = NULL;
            printf("\n贪心算法详细结果已追加到 %s\n", filepath);
        }
        else {
            printf("无法追加到文件 %s (错误代码: %d)\n", filepath, errno);
        }
    }

    // 释放内存
    free(indices);
    free(densities);
    free(selected);
}

// 回溯法辅助函数
void backtrackRecursive(int level, double current_weight, double current_value,
    double* max_val, int* selected, int* best_selected) {
    if (level == item_count) {
        if (current_value > *max_val) {
            *max_val = current_value;
            for (int i = 0; i < item_count; i++) {
                best_selected[i] = selected[i];
            }
        }
        return;
    }

    // 剪枝：如果当前价值加上剩余物品的最大可能价值仍小于已知最大值，则剪枝
    double remaining_bound = current_value;
    double remaining_capacity = knapsack_capacity - current_weight;
    for (int i = level; i < item_count && remaining_capacity > 0; i++) {
        if (items[i].weight <= remaining_capacity) {
            remaining_bound += items[i].value;
            remaining_capacity -= items[i].weight;
        }
        else {
            remaining_bound += items[i].value * (remaining_capacity / items[i].weight);
            break;
        }
    }
    if (remaining_bound <= *max_val) return;

    // 优先探索更有希望的路径（按价值密度排序）
    if (items[level].weight <= knapsack_capacity - current_weight) {
        selected[level] = 1;
        backtrackRecursive(level + 1,
            current_weight + items[level].weight,
            current_value + items[level].value,
            max_val, selected, best_selected);
    }

    selected[level] = 0;
    backtrackRecursive(level + 1, current_weight, current_value, max_val, selected, best_selected);
}

// 回溯法
void solveWithBacktracking() {
    // 按价值密度降序排序
    for (int i = 0; i < item_count - 1; i++) {
        for (int j = i + 1; j < item_count; j++) {
            double density_i = items[i].value / items[i].weight;
            double density_j = items[j].value / items[j].weight;
            if (density_i < density_j) {
                Item temp = items[i];
                items[i] = items[j];
                items[j] = temp;
            }
        }
    }

    clock_t start = clock();

    int* selected = (int*)calloc(item_count, sizeof(int));
    int* best_selected = (int*)calloc(item_count, sizeof(int));
    if (!selected || !best_selected) {
        printf("内存分配失败!\n");
        free(selected);
        free(best_selected);
        return;
    }

    double current_max = 0;
    backtrackRecursive(0, 0, 0, &current_max, selected, best_selected);
    max_value = current_max;

    clock_t end = clock();
    double time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    // 打印结果
    printf("\n回溯法(带剪枝)求解结果:\n");
    printf("最大价值: %.2f\n", max_value);
    printf("计算时间: %.2f 毫秒\n", time_used);

    printf("\n选择的物品详细信息:\n");
    printf("物品编号\t重量\t价值\n");
    double total_weight = 0;
    for (int i = 0; i < item_count; i++) {
        if (best_selected[i]) {
            printf("%d\t\t%d\t%.2f\n", i + 1, items[i].weight, items[i].value);
            total_weight += items[i].weight;
        }
    }
    printf("\n总重量: %.2f (剩余容量: %.2f)\n", total_weight, knapsack_capacity - total_weight);

    // 保存详细结果
    if (item_count == 1000 && knapsack_capacity == 5000) {
        char filepath[256];
        getOutputFilePath(filepath, sizeof(filepath), "sp_results.csv");

        result_file = fopen(filepath, "a");  // 追加模式
        if (result_file) {
            fprintf(result_file, "\n\n算法类型,回溯法\n");
            fprintf(result_file, "物品数量,%d\n", item_count);
            fprintf(result_file, "背包容量,%d\n\n", knapsack_capacity);
            fprintf(result_file, "最大价值,%.2f\n", max_value);
            fprintf(result_file, "计算时间(毫秒),%.2f\n", time_used);

            fprintf(result_file, "\n物品编号,重量,价值,是否选中\n");
            for (int i = 0; i < item_count; i++) {
                fprintf(result_file, "%d,%d,%.2f,%d\n",
                    i + 1, items[i].weight, items[i].value, best_selected[i]);
            }
            fclose(result_file);
            result_file = NULL;
            printf("\n回溯法详细结果已追加到 %s\n", filepath);
        }
        else {
            printf("无法追加到文件 %s (错误代码: %d)\n", filepath, errno);
        }
    }

    // 释放内存
    free(selected);
    free(best_selected);
}

// 输入背包容量
void inputKnapsackCapacity() {
    char input[20];
    while (1) {
        printf("请输入背包容量(1-%d): ", MAX_W);
        if (fgets(input, sizeof(input), stdin)) {
            input[strcspn(input, "\n")] = '\0';
            if (!isValidNumber(input)) {
                printf("错误: 请输入有效的正整数!\n");
                continue;
            }
            knapsack_capacity = atoi(input);
            if (knapsack_capacity <= 0 || knapsack_capacity > MAX_W) {
                printf("错误: 背包容量必须在1到%d之间!\n", MAX_W);
                continue;
            }
            break;
        }
    }
}

// 输入物品数量
void inputItemCount() {
    char input[20];
    while (1) {
        printf("请输入物品数量(1-%d): ", MAX_N);
        if (fgets(input, sizeof(input), stdin) != NULL) {
            input[strcspn(input, "\n")] = '\0';
            if (!isValidNumber(input)) {
                printf("错误: 请输入有效的正整数!\n");
                continue;
            }
            item_count = atoi(input);
            if (item_count <= 0 || item_count > MAX_N) {
                printf("错误: 物品数量必须在1到%d之间!\n", MAX_N);
                continue;
            }
            break;
        }
    }
}

// 输入物品信息
void inputItems() {
    char input[50];
    int weight, value;
    printf("请依次输入每个物品的重量和价值(格式: 重量,价值):\n");
    for (int i = 0; i < item_count; i++) {
        while (1) {
            printf("物品%d: ", i + 1);
            if (fgets(input, sizeof(input), stdin) != NULL) {
                input[strcspn(input, "\n")] = '\0';
                if (!parseItemInput(input, &weight, &value)) {
                    printf("错误: 请输入有效的重量和价值(格式: 重量,价值)!\n");
                    continue;
                }
                items[i].weight = weight;
                items[i].value = (double)value;
                break;
            }
        }
    }
}

// 生成随机物品数据
void generateRandomItems() {
    srand(time(NULL));
    for (int i = 0; i < item_count; i++) {
        // 重量范围1-100，确保不为0
        items[i].weight = 1 + rand() % 100;
        // 价值范围100-999.99
        items[i].value = 100 + (rand() % 900) + (rand() % 100) / 100.0;
    }
}

// 运行性能测试
void runPerformanceTests() {
    int test_sizes[] = { 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000 };
    int test_capacities[] = { 100, 1000, 5000, 10000, 100000 };
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    int num_caps = sizeof(test_capacities) / sizeof(test_capacities[0]);

    printf("\n性能测试开始...\n");
    printf("| %-10s | %-10s | %-15s | %-15s | %-15s | %-15s | %-15s | %-15s | %-15s | %-15s |\n",
        "物品数量", "背包容量", "蛮力法价值", "蛮力法时间",
        "动态规划价值", "动态规划时间", "贪心算法价值", "贪心算法时间",
        "回溯法价值", "回溯法时间");
    printf("|-----------|-----------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|\n");

    char filepath[256];
    getOutputFilePath(filepath, sizeof(filepath), "performance_results.csv");

    FILE* fp = fopen(filepath, "w");
    if (fp) {
        // 写入表头
        fprintf(fp, "物品数量,背包容量,蛮力法价值,蛮力法时间(ms),动态规划价值,动态规划时间(ms),贪心算法价值,贪心算法时间(ms),回溯法价值,回溯法时间(ms)\n");

        for (int i = 0; i < num_sizes; i++) {
            for (int j = 0; j < num_caps; j++) {
                item_count = test_sizes[i];
                knapsack_capacity = test_capacities[j];
                generateRandomItems();

                double brute_value = -1, dp_value = -1, greedy_value = -1, backtrack_value = -1;
                double brute_time = -1, dp_time = -1, greedy_time = -1, backtrack_time = -1;

                // 保存原始最大值
                double original_max_value = max_value;

                // 蛮力法测试
                clock_t brute_start = clock();
                if (item_count <= 25) { // 只对小型数据集使用蛮力法
                    solveWithBruteForce();
                    brute_value = max_value;
                }
                brute_time = ((double)(clock() - brute_start)) / CLOCKS_PER_SEC * 1000;
                max_value = original_max_value;

                // 动态规划测试
                if (knapsack_capacity <= MAX_DP_CAPACITY) {
                    clock_t dp_start = clock();
                    solveWithDynamicProgramming();
                    dp_value = max_value;
                    dp_time = ((double)(clock() - dp_start)) / CLOCKS_PER_SEC * 1000;
                }
                max_value = original_max_value;

                // 贪心算法测试
                clock_t greedy_start = clock();
                solveWithGreedy();
                greedy_value = max_value;
                greedy_time = ((double)(clock() - greedy_start)) / CLOCKS_PER_SEC * 1000;
                max_value = original_max_value;

                // 回溯法测试
                clock_t backtrack_start = clock();
                if (item_count <= 50) { // 只对小型数据集使用回溯法
                    solveWithBacktracking();
                    backtrack_value = max_value;
                }
                backtrack_time = ((double)(clock() - backtrack_start)) / CLOCKS_PER_SEC * 1000;
                max_value = original_max_value;

                // 打印结果到控制台
                printf("| %-10d | %-10d |", item_count, knapsack_capacity);
                if (brute_value == -1) printf(" %-15s |", "N/A");
                else printf(" %-15.2f |", brute_value);
                if (brute_time == -1) printf(" %-15s |", "N/A");
                else printf(" %-15.3f |", brute_time);

                if (dp_value == -1) printf(" %-15s |", "N/A");
                else printf(" %-15.2f |", dp_value);
                if (dp_time == -1) printf(" %-15s |", "N/A");
                else printf(" %-15.3f |", dp_time);

                printf(" %-15.2f | %-15.3f |", greedy_value, greedy_time);

                if (backtrack_value == -1) printf(" %-15s |", "N/A");
                else printf(" %-15.2f |", backtrack_value);
                if (backtrack_time == -1) printf(" %-15s |\n", "N/A");
                else printf(" %-15.3f |\n", backtrack_time);

                // 写入结果到CSV文件
                fprintf(fp, "%d,%d,%.2f,%.3f,%.2f,%.3f,%.2f,%.3f,%.2f,%.3f\n",
                    item_count, knapsack_capacity,
                    brute_value, brute_time,
                    dp_value, dp_time,
                    greedy_value, greedy_time,
                    backtrack_value, backtrack_time);

                printf("已完成测试: 物品数量=%d, 背包容量=%d\n", item_count, knapsack_capacity);
            }
        }

        fclose(fp);
        printf("\n测试结果已保存到 %s\n", filepath);
    }
    else {
        printf("无法创建性能测试文件 %s (错误代码: %d)\n", filepath, errno);
    }
}


int main() {
    ensureOutputDirectory();

    printf("=== 0-1背包问题求解程序 ===\n");
    printf("支持物品数量最多%d个，背包容量最大%d\n", MAX_N, MAX_W);
    printf("所有输出文件将保存到 'output' 目录\n");

    int choice;
    while (1) {
        printf("\n主菜单:\n");
        printf("1. 手动输入数据求解\n");
        printf("2. 运行性能测试\n");
        printf("3. 退出程序\n");
        printf("请选择操作: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("无效的输入! 请输入数字1-3。\n");
            continue;
        }
        clearInputBuffer();

        switch (choice) {
        case 1: {
            inputKnapsackCapacity();
            inputItemCount();
            inputItems();
            saveItemsToCSV("items_info.csv");

            int algoChoice;
            do {
                printf("\n算法选择菜单:\n");
                printf("1. 蛮力法求解\n");
                printf("2. 动态规划法求解\n");
                printf("3. 贪心算法求解\n");
                printf("4. 回溯法求解\n");
                printf("5. 显示物品信息\n");
                printf("6. 重新输入数据\n");
                printf("7. 返回主菜单\n");
                printf("请选择操作: ");

                if (scanf("%d", &algoChoice) != 1) {
                    clearInputBuffer();
                    printf("无效的输入! 请输入数字1-7。\n");
                    continue;
                }
                clearInputBuffer();

                switch (algoChoice) {
                case 1: solveWithBruteForce(); break;
                case 2: solveWithDynamicProgramming(); break;
                case 3: solveWithGreedy(); break;
                case 4: solveWithBacktracking(); break;
                case 5: displayItems(); break;
                case 6:
                    inputKnapsackCapacity();
                    inputItemCount();
                    inputItems();
                    saveItemsToCSV("items_info.csv");
                    break;
                case 7: break;
                default: printf("无效的选择! 请输入数字1-7。\n");
                }
            } while (algoChoice != 7);
            break;
        }
        case 2:
            runPerformanceTests();
            break;
        case 3:
            printf("程序退出。\n");
            return 0;
        default:
            printf("无效的选择! 请输入数字1-3。\n");
        }
    }
    return 0;
}