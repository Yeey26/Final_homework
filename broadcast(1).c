#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>
#include <direct.h>  // Windows Ŀ¼����
#include <errno.h>   // ������

#define MAX_N 320000      // �����Ʒ����
#define MAX_W 1000000     // ��󱳰�����
#define MAX_DP_CAPACITY 100000 // ��̬�滮�������������

// �������Ŀ¼����
void ensureOutputDirectory() {
    // ���Ŀ¼�Ƿ����
    if (_access("output", 0) == -1) {
        // Ŀ¼�����ڣ����Դ���
        if (_mkdir("output") != 0) {
            printf("�޷��������Ŀ¼ 'output'���������: %d\n", errno);
        }
        else {
            printf("�Ѵ������Ŀ¼ 'output\\'\n");
        }
    }
}

// ��ȡ��·�����ļ���
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

// ��������
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

// ������뻺����
void clearInputBuffer() {
    while (getchar() != '\n');
}

// ����ַ����Ƿ�ֻ��������
bool isValidNumber(const char* str) {
    if (str == NULL || *str == '\0') return false;
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}

// �������ŷָ�������
bool parseItemInput(const char* input, int* weight, int* value) {
    char inputCopy[50];
    strncpy(inputCopy, input, sizeof(inputCopy));
    inputCopy[sizeof(inputCopy) - 1] = '\0';

    char* token = strtok(inputCopy, ",");
    if (token == NULL) {
        // �������Ķ���
        strncpy(inputCopy, input, sizeof(inputCopy));
        token = strtok(inputCopy, "��");
    }
    if (token == NULL) return false;

    char* endptr;
    *weight = strtol(token, &endptr, 10);
    if (*endptr != '\0' || *weight <= 0) return false;

    token = strtok(NULL, ",");
    if (token == NULL) {
        // �������Ķ���
        token = strtok(NULL, "��");
    }
    if (token == NULL) return false;

    *value = strtol(token, &endptr, 10);
    if (*endptr != '\0' || *value <= 0) return false;

    return true;
}

// ��ʾ��ǰ��Ʒ��Ϣ
void displayItems() {
    printf("\n��ǰ��Ʒ��Ϣ:\n");
    printf("��Ʒ����: %d\n", item_count);
    printf("��������: %d\n", knapsack_capacity);
    printf("��Ʒ���\t����\t��ֵ\t��ֵ�ܶ�\n");

    int displayCount = item_count > 1000 ? 1000 : item_count;
    for (int i = 0; i < displayCount; i++) {
        double density = items[i].value / items[i].weight;
        printf("%d\t\t%d\t%.2f\t%.2f\n", i + 1, items[i].weight, items[i].value, density);
    }
    if (item_count > 1000) {
        printf("... (��%d����Ʒ��ֻ��ʾǰ1000��)\n", item_count);
    }
}

// ������Ʒ��Ϣ���ļ�
void saveItemsToCSV(const char* filename) {
    char filepath[256];
    getOutputFilePath(filepath, sizeof(filepath), filename);

    FILE* fp = fopen(filepath, "w");
    if (fp == NULL) {
        printf("�޷������ļ� %s (�������: %d)\n", filepath, errno);
        return;
    }

    fprintf(fp, "��Ʒ���,����,��ֵ\n");
    int saveCount = item_count > 1000 ? 1000 : item_count;
    for (int i = 0; i < saveCount; i++) {
        fprintf(fp, "%d,%d,%.2f\n", i + 1, items[i].weight, items[i].value);
    }
    fclose(fp);
    printf("��Ʒ��Ϣ�ѱ��浽 %s\n", filepath);
}

// ��̬�滮�����
void solveWithDynamicProgramming() {
    if (knapsack_capacity > MAX_DP_CAPACITY) {
        printf("\n��̬�滮�������:\n");
        printf("����: ������������(%d > %d)����̬�滮���������Ĺ����ڴ�!\n",
            knapsack_capacity, MAX_DP_CAPACITY);
        printf("����ʹ�������㷨���С����������\n");
        return;
    }

    clock_t startTime = clock();

    // ���䶯̬�滮���ڴ�
    double** dp = (double**)malloc((item_count + 1) * sizeof(double*));
    for (int i = 0; i <= item_count; i++) {
        dp[i] = (double*)malloc((knapsack_capacity + 1) * sizeof(double));
        memset(dp[i], 0, (knapsack_capacity + 1) * sizeof(double));
    }

    // ��䶯̬�滮��
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

    // �������ѡ�����Ʒ
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

    // ��ӡ���
    printf("\n��̬�滮�������:\n");
    printf("����ֵ: %.2f\n", max_value);
    printf("����ʱ��: %.2f ����\n", time_used);

    // ��ӡѡ�����Ʒ��Ϣ
    printf("ѡ�����Ʒ���: ");
    for (int i = 0; i < item_count; i++) {
        if (selected[i]) {
            printf("%d ", i + 1);
        }
    }

    printf("\n\nѡ�����Ʒ��ϸ��Ϣ:\n");
    printf("��Ʒ���\t����\t��ֵ\n");
    for (int i = 0; i < item_count; i++) {
        if (selected[i]) {
            printf("%d\t\t%d\t%.2f\n", i + 1, items[i].weight, items[i].value);
        }
    }
    printf("\n������: %.2f (ʣ������: %.2f)\n", total_weight, knapsack_capacity - total_weight);

    // ������ϸ���
    if (item_count == 1000 && knapsack_capacity == 5000) {
        char filepath[256];
        getOutputFilePath(filepath, sizeof(filepath), "sp_results.csv");

        result_file = fopen(filepath, "w");
        if (result_file) {
            fprintf(result_file, "�㷨����,��̬�滮��\n");
            fprintf(result_file, "��Ʒ����,%d\n", item_count);
            fprintf(result_file, "��������,%d\n\n", knapsack_capacity);
            fprintf(result_file, "����ֵ,%.2f\n", max_value);
            fprintf(result_file, "����ʱ��(����),%.2f\n", time_used);

            fprintf(result_file, "\n��Ʒ���,����,��ֵ,�Ƿ�ѡ��\n");
            for (int i = 0; i < item_count; i++) {
                fprintf(result_file, "%d,%d,%.2f,%d\n",
                    i + 1, items[i].weight, items[i].value, selected[i]);
            }
            fclose(result_file);
            result_file = NULL;
            printf("\n��̬�滮����ϸ����ѱ��浽 %s\n", filepath);
        }
        else {
            printf("�޷������ļ� %s (�������: %d)\n", filepath, errno);
        }
    }

    // �ͷ��ڴ�
    for (int i = 0; i <= item_count; i++) {
        free(dp[i]);
    }
    free(dp);
    free(selected);
}

// ���������
void solveWithBruteForce() {
    clock_t start = clock();
    double max_val = 0;
    int best_combination = 0;

    // �������п��ܵ���� (2^n�ֿ���)
    for (int i = 0; i < (1 << item_count); i++) {
        double total_weight = 0;
        double total_value = 0;

        for (int j = 0; j < item_count; j++) {
            if (i & (1 << j)) {
                total_weight += items[j].weight;
                total_value += items[j].value;
            }
        }

        // �����ǰ����������������Ҽ�ֵ������������ֵ
        if (total_weight <= knapsack_capacity && total_value > max_val) {
            max_val = total_value;
            best_combination = i;
        }
    }

    max_value = max_val;
    clock_t end = clock();
    double time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    // ��ӡ���
    printf("\n�����������:\n");
    printf("����ֵ: %.2f\n", max_value);
    printf("����ʱ��: %.2f ����\n", time_used);

    // ��ʾѡ�����Ʒ��ϸ��Ϣ
    printf("\nѡ�����Ʒ��ϸ��Ϣ:\n");
    printf("��Ʒ���\t����\t��ֵ\n");
    double total_w = 0;
    int* selected = (int*)calloc(item_count, sizeof(int));
    for (int i = 0; i < item_count; i++) {
        if (best_combination & (1 << i)) {
            printf("%d\t\t%d\t%.2f\n", i + 1, items[i].weight, items[i].value);
            total_w += items[i].weight;
            selected[i] = 1;
        }
    }
    printf("\n������: %.2f (ʣ������: %.2f)\n", total_w, knapsack_capacity - total_w);

    // ������ϸ���
    if (item_count == 1000 && knapsack_capacity == 5000) {
        char filepath[256];
        getOutputFilePath(filepath, sizeof(filepath), "sp_results.csv");

        result_file = fopen(filepath, "a");  // ׷��ģʽ
        if (result_file) {
            fprintf(result_file, "\n\n�㷨����,������\n");
            fprintf(result_file, "��Ʒ����,%d\n", item_count);
            fprintf(result_file, "��������,%d\n\n", knapsack_capacity);
            fprintf(result_file, "����ֵ,%.2f\n", max_value);
            fprintf(result_file, "����ʱ��(����),%.2f\n", time_used);

            fprintf(result_file, "\n��Ʒ���,����,��ֵ,�Ƿ�ѡ��\n");
            for (int i = 0; i < item_count; i++) {
                fprintf(result_file, "%d,%d,%.2f,%d\n",
                    i + 1, items[i].weight, items[i].value, selected[i]);
            }
            fclose(result_file);
            result_file = NULL;
            printf("\n��������ϸ�����׷�ӵ� %s\n", filepath);
        }
        else {
            printf("�޷�׷�ӵ��ļ� %s (�������: %d)\n", filepath, errno);
        }
    }

    free(selected);
}

// ������������
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

// ������
void heapSortDensities(double densities[], int indices[], int n) {
    // ��������
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(densities, indices, n, i);

    // ��ȡԪ��
    for (int i = n - 1; i > 0; i--) {
        // �ƶ���ǰ����ĩβ
        double temp = densities[0];
        densities[0] = densities[i];
        densities[i] = temp;

        int temp_idx = indices[0];
        indices[0] = indices[i];
        indices[i] = temp_idx;

        // �ڼ��ٵĶ��ϵ���heapify
        heapify(densities, indices, i, 0);
    }
}

// ̰���㷨
void solveWithGreedy() {
    if (item_count <= 0 || knapsack_capacity <= 0) {
        printf("��Ч����Ʒ�����򱳰�����!\n");
        return;
    }

    clock_t start = clock();

    // �����������鲢�����ֵ�ܶ�
    int* indices = (int*)malloc(item_count * sizeof(int));
    double* densities = (double*)malloc(item_count * sizeof(double));
    if (!indices || !densities) {
        printf("�ڴ����ʧ��!\n");
        free(indices);
        free(densities);
        return;
    }

    for (int i = 0; i < item_count; i++) {
        indices[i] = i;
        densities[i] = items[i].value / items[i].weight;
    }

    // ʹ�ö����򰴼�ֵ�ܶȽ�������
    heapSortDensities(densities, indices, item_count);

    double current_weight = 0;
    max_value = 0;
    int* selected = (int*)calloc(item_count, sizeof(int));
    if (!selected) {
        printf("�ڴ����ʧ��!\n");
        free(indices);
        free(densities);
        return;
    }

    // ѡ����Ʒ���Ӽ�ֵ�ܶ���ߵĿ�ʼ��
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

    // ��ӡ���
    printf("\n̰���㷨�����:\n");
    printf("����ֵ: %.2f\n", max_value);
    printf("����ʱ��: %.2f ����\n", time_used);

    printf("\nѡ�����Ʒ��ϸ��Ϣ:\n");
    printf("��Ʒ���\t����\t��ֵ\n");
    for (int i = 0; i < item_count; i++) {
        if (selected[i]) {
            printf("%d\t\t%d\t%.2f\n", i + 1, items[i].weight, items[i].value);
        }
    }
    printf("\n������: %.2f (ʣ������: %.2f)\n", current_weight, knapsack_capacity - current_weight);

    // ������ϸ���
    if (item_count == 1000 && knapsack_capacity == 5000) {
        char filepath[256];
        getOutputFilePath(filepath, sizeof(filepath), "sp_results.csv");

        result_file = fopen(filepath, "a");  // ׷��ģʽ
        if (result_file) {
            fprintf(result_file, "\n\n�㷨����,̰���㷨\n");
            fprintf(result_file, "��Ʒ����,%d\n", item_count);
            fprintf(result_file, "��������,%d\n\n", knapsack_capacity);
            fprintf(result_file, "����ֵ,%.2f\n", max_value);
            fprintf(result_file, "����ʱ��(����),%.2f\n", time_used);

            fprintf(result_file, "\n��Ʒ���,����,��ֵ,�Ƿ�ѡ��\n");
            for (int i = 0; i < item_count; i++) {
                fprintf(result_file, "%d,%d,%.2f,%d\n",
                    i + 1, items[i].weight, items[i].value, selected[i]);
            }
            fclose(result_file);
            result_file = NULL;
            printf("\n̰���㷨��ϸ�����׷�ӵ� %s\n", filepath);
        }
        else {
            printf("�޷�׷�ӵ��ļ� %s (�������: %d)\n", filepath, errno);
        }
    }

    // �ͷ��ڴ�
    free(indices);
    free(densities);
    free(selected);
}

// ���ݷ���������
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

    // ��֦�������ǰ��ֵ����ʣ����Ʒ�������ܼ�ֵ��С����֪���ֵ�����֦
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

    // ����̽������ϣ����·��������ֵ�ܶ�����
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

// ���ݷ�
void solveWithBacktracking() {
    // ����ֵ�ܶȽ�������
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
        printf("�ڴ����ʧ��!\n");
        free(selected);
        free(best_selected);
        return;
    }

    double current_max = 0;
    backtrackRecursive(0, 0, 0, &current_max, selected, best_selected);
    max_value = current_max;

    clock_t end = clock();
    double time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    // ��ӡ���
    printf("\n���ݷ�(����֦)�����:\n");
    printf("����ֵ: %.2f\n", max_value);
    printf("����ʱ��: %.2f ����\n", time_used);

    printf("\nѡ�����Ʒ��ϸ��Ϣ:\n");
    printf("��Ʒ���\t����\t��ֵ\n");
    double total_weight = 0;
    for (int i = 0; i < item_count; i++) {
        if (best_selected[i]) {
            printf("%d\t\t%d\t%.2f\n", i + 1, items[i].weight, items[i].value);
            total_weight += items[i].weight;
        }
    }
    printf("\n������: %.2f (ʣ������: %.2f)\n", total_weight, knapsack_capacity - total_weight);

    // ������ϸ���
    if (item_count == 1000 && knapsack_capacity == 5000) {
        char filepath[256];
        getOutputFilePath(filepath, sizeof(filepath), "sp_results.csv");

        result_file = fopen(filepath, "a");  // ׷��ģʽ
        if (result_file) {
            fprintf(result_file, "\n\n�㷨����,���ݷ�\n");
            fprintf(result_file, "��Ʒ����,%d\n", item_count);
            fprintf(result_file, "��������,%d\n\n", knapsack_capacity);
            fprintf(result_file, "����ֵ,%.2f\n", max_value);
            fprintf(result_file, "����ʱ��(����),%.2f\n", time_used);

            fprintf(result_file, "\n��Ʒ���,����,��ֵ,�Ƿ�ѡ��\n");
            for (int i = 0; i < item_count; i++) {
                fprintf(result_file, "%d,%d,%.2f,%d\n",
                    i + 1, items[i].weight, items[i].value, best_selected[i]);
            }
            fclose(result_file);
            result_file = NULL;
            printf("\n���ݷ���ϸ�����׷�ӵ� %s\n", filepath);
        }
        else {
            printf("�޷�׷�ӵ��ļ� %s (�������: %d)\n", filepath, errno);
        }
    }

    // �ͷ��ڴ�
    free(selected);
    free(best_selected);
}

// ���뱳������
void inputKnapsackCapacity() {
    char input[20];
    while (1) {
        printf("�����뱳������(1-%d): ", MAX_W);
        if (fgets(input, sizeof(input), stdin)) {
            input[strcspn(input, "\n")] = '\0';
            if (!isValidNumber(input)) {
                printf("����: ��������Ч��������!\n");
                continue;
            }
            knapsack_capacity = atoi(input);
            if (knapsack_capacity <= 0 || knapsack_capacity > MAX_W) {
                printf("����: ��������������1��%d֮��!\n", MAX_W);
                continue;
            }
            break;
        }
    }
}

// ������Ʒ����
void inputItemCount() {
    char input[20];
    while (1) {
        printf("��������Ʒ����(1-%d): ", MAX_N);
        if (fgets(input, sizeof(input), stdin) != NULL) {
            input[strcspn(input, "\n")] = '\0';
            if (!isValidNumber(input)) {
                printf("����: ��������Ч��������!\n");
                continue;
            }
            item_count = atoi(input);
            if (item_count <= 0 || item_count > MAX_N) {
                printf("����: ��Ʒ����������1��%d֮��!\n", MAX_N);
                continue;
            }
            break;
        }
    }
}

// ������Ʒ��Ϣ
void inputItems() {
    char input[50];
    int weight, value;
    printf("����������ÿ����Ʒ�������ͼ�ֵ(��ʽ: ����,��ֵ):\n");
    for (int i = 0; i < item_count; i++) {
        while (1) {
            printf("��Ʒ%d: ", i + 1);
            if (fgets(input, sizeof(input), stdin) != NULL) {
                input[strcspn(input, "\n")] = '\0';
                if (!parseItemInput(input, &weight, &value)) {
                    printf("����: ��������Ч�������ͼ�ֵ(��ʽ: ����,��ֵ)!\n");
                    continue;
                }
                items[i].weight = weight;
                items[i].value = (double)value;
                break;
            }
        }
    }
}

// ���������Ʒ����
void generateRandomItems() {
    srand(time(NULL));
    for (int i = 0; i < item_count; i++) {
        // ������Χ1-100��ȷ����Ϊ0
        items[i].weight = 1 + rand() % 100;
        // ��ֵ��Χ100-999.99
        items[i].value = 100 + (rand() % 900) + (rand() % 100) / 100.0;
    }
}

// �������ܲ���
void runPerformanceTests() {
    int test_sizes[] = { 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000 };
    int test_capacities[] = { 100, 1000, 5000, 10000, 100000 };
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    int num_caps = sizeof(test_capacities) / sizeof(test_capacities[0]);

    printf("\n���ܲ��Կ�ʼ...\n");
    printf("| %-10s | %-10s | %-15s | %-15s | %-15s | %-15s | %-15s | %-15s | %-15s | %-15s |\n",
        "��Ʒ����", "��������", "��������ֵ", "������ʱ��",
        "��̬�滮��ֵ", "��̬�滮ʱ��", "̰���㷨��ֵ", "̰���㷨ʱ��",
        "���ݷ���ֵ", "���ݷ�ʱ��");
    printf("|-----------|-----------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|\n");

    char filepath[256];
    getOutputFilePath(filepath, sizeof(filepath), "performance_results.csv");

    FILE* fp = fopen(filepath, "w");
    if (fp) {
        // д���ͷ
        fprintf(fp, "��Ʒ����,��������,��������ֵ,������ʱ��(ms),��̬�滮��ֵ,��̬�滮ʱ��(ms),̰���㷨��ֵ,̰���㷨ʱ��(ms),���ݷ���ֵ,���ݷ�ʱ��(ms)\n");

        for (int i = 0; i < num_sizes; i++) {
            for (int j = 0; j < num_caps; j++) {
                item_count = test_sizes[i];
                knapsack_capacity = test_capacities[j];
                generateRandomItems();

                double brute_value = -1, dp_value = -1, greedy_value = -1, backtrack_value = -1;
                double brute_time = -1, dp_time = -1, greedy_time = -1, backtrack_time = -1;

                // ����ԭʼ���ֵ
                double original_max_value = max_value;

                // ����������
                clock_t brute_start = clock();
                if (item_count <= 25) { // ֻ��С�����ݼ�ʹ��������
                    solveWithBruteForce();
                    brute_value = max_value;
                }
                brute_time = ((double)(clock() - brute_start)) / CLOCKS_PER_SEC * 1000;
                max_value = original_max_value;

                // ��̬�滮����
                if (knapsack_capacity <= MAX_DP_CAPACITY) {
                    clock_t dp_start = clock();
                    solveWithDynamicProgramming();
                    dp_value = max_value;
                    dp_time = ((double)(clock() - dp_start)) / CLOCKS_PER_SEC * 1000;
                }
                max_value = original_max_value;

                // ̰���㷨����
                clock_t greedy_start = clock();
                solveWithGreedy();
                greedy_value = max_value;
                greedy_time = ((double)(clock() - greedy_start)) / CLOCKS_PER_SEC * 1000;
                max_value = original_max_value;

                // ���ݷ�����
                clock_t backtrack_start = clock();
                if (item_count <= 50) { // ֻ��С�����ݼ�ʹ�û��ݷ�
                    solveWithBacktracking();
                    backtrack_value = max_value;
                }
                backtrack_time = ((double)(clock() - backtrack_start)) / CLOCKS_PER_SEC * 1000;
                max_value = original_max_value;

                // ��ӡ���������̨
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

                // д������CSV�ļ�
                fprintf(fp, "%d,%d,%.2f,%.3f,%.2f,%.3f,%.2f,%.3f,%.2f,%.3f\n",
                    item_count, knapsack_capacity,
                    brute_value, brute_time,
                    dp_value, dp_time,
                    greedy_value, greedy_time,
                    backtrack_value, backtrack_time);

                printf("����ɲ���: ��Ʒ����=%d, ��������=%d\n", item_count, knapsack_capacity);
            }
        }

        fclose(fp);
        printf("\n���Խ���ѱ��浽 %s\n", filepath);
    }
    else {
        printf("�޷��������ܲ����ļ� %s (�������: %d)\n", filepath, errno);
    }
}


int main() {
    ensureOutputDirectory();

    printf("=== 0-1�������������� ===\n");
    printf("֧����Ʒ�������%d���������������%d\n", MAX_N, MAX_W);
    printf("��������ļ������浽 'output' Ŀ¼\n");

    int choice;
    while (1) {
        printf("\n���˵�:\n");
        printf("1. �ֶ������������\n");
        printf("2. �������ܲ���\n");
        printf("3. �˳�����\n");
        printf("��ѡ�����: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("��Ч������! ����������1-3��\n");
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
                printf("\n�㷨ѡ��˵�:\n");
                printf("1. ���������\n");
                printf("2. ��̬�滮�����\n");
                printf("3. ̰���㷨���\n");
                printf("4. ���ݷ����\n");
                printf("5. ��ʾ��Ʒ��Ϣ\n");
                printf("6. ������������\n");
                printf("7. �������˵�\n");
                printf("��ѡ�����: ");

                if (scanf("%d", &algoChoice) != 1) {
                    clearInputBuffer();
                    printf("��Ч������! ����������1-7��\n");
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
                default: printf("��Ч��ѡ��! ����������1-7��\n");
                }
            } while (algoChoice != 7);
            break;
        }
        case 2:
            runPerformanceTests();
            break;
        case 3:
            printf("�����˳���\n");
            return 0;
        default:
            printf("��Ч��ѡ��! ����������1-3��\n");
        }
    }
    return 0;
}