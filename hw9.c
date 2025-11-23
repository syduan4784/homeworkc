#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 200
#define RUNS 1000   // 각 정렬 알고리즘 반복 횟수

// --------------------------- 학생 구조체 정의 ---------------------------

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char gender;   // 'M' / 'F' 등
    int korean;
    int english;
    int math;
} Student;

// --------------------------- 파일 로드 함수 ---------------------------
// CSV 파일에서 학생 정보를 읽어 Student 배열로 변환한다.

Student* load_students(const char* filename, int* out_count) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }

    char line[MAX_LINE_LEN];
    int capacity = 10;
    int count = 0;
    Student* arr = malloc(sizeof(Student) * capacity);

    if (!arr) {
        perror("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    // 첫 줄 헤더 스킵
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        if (count >= capacity) {
            capacity *= 2;
            Student* temp = realloc(arr, sizeof(Student) * capacity);
            if (!temp) {
                perror("Reallocation failed");
                free(arr);
                fclose(fp);
                return NULL;
            }
            arr = temp;
        }

        Student s;
        char* token = strtok(line, ",");
        if (!token) continue;
        s.id = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(s.name, token, MAX_NAME_LEN);
        s.name[MAX_NAME_LEN - 1] = '\0';

        token = strtok(NULL, ",");
        if (!token) continue;
        s.gender = token[0];

        token = strtok(NULL, ",");
        if (!token) continue;
        s.korean = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.english = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.math = atoi(token);

        arr[count++] = s;
    }

    fclose(fp);

    // 파일 읽기 완료 → 사용한 만큼만 메모리 딱 맞게 조정
    Student* tight = realloc(arr, sizeof(Student) * count);
    if (!tight) {
        fprintf(stderr, "Warning: Tight reallocation failed, using original memory.\n");
        *out_count = count;
        return arr;
    }

    *out_count = count;
    return tight;
}

// --------------------------- 유틸리티 함수 ---------------------------

// 학생 배열 복사
void copy_students(Student* src, Student* dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

// 총점 계산 (국어 + 영어 + 수학)
int total_grade(const Student* s) {
    return s->korean + s->english + s->math;
}

// --------------------------- 비교 함수 타입 & 선언 ---------------------------

typedef int (*CompareFunc)(const Student*, const Student*, long long*);

// ID 기준
int cmp_id_asc(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;
    if (a->id < b->id) return -1;
    if (a->id > b->id) return 1;
    return 0;
}
int cmp_id_desc(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;
    if (a->id > b->id) return -1;
    if (a->id < b->id) return 1;
    return 0;
}

// NAME 기준 (사전순)
int cmp_name_asc(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;
    int r = strcmp(a->name, b->name);
    if (r < 0) return -1;
    if (r > 0) return 1;
    return 0;
}
int cmp_name_desc(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;
    int r = strcmp(a->name, b->name);
    if (r < 0) return 1;
    if (r > 0) return -1;
    return 0;
}

// GENDER 기준 (문자 코드 기준, 예: 'F' < 'M')
int cmp_gender_asc(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;
    if (a->gender < b->gender) return -1;
    if (a->gender > b->gender) return 1;
    return 0;
}
int cmp_gender_desc(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;
    if (a->gender > b->gender) return -1;
    if (a->gender < b->gender) return 1;
    return 0;
}

// GRADE(총점 + tie-break) 기준
int cmp_grade_core_asc(const Student* a, const Student* b) {
    int ta = total_grade(a);
    int tb = total_grade(b);

    if (ta < tb) return -1;
    if (ta > tb) return 1;

    // 총점이 같을 경우: 국어 → 영어 → 수학
    if (a->korean < b->korean) return -1;
    if (a->korean > b->korean) return 1;

    if (a->english < b->english) return -1;
    if (a->english > b->english) return 1;

    if (a->math < b->math) return -1;
    if (a->math > b->math) return 1;

    return 0;
}

int cmp_grade_asc(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;
    return cmp_grade_core_asc(a, b);
}
int cmp_grade_desc(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;
    int r = cmp_grade_core_asc(a, b);
    return -r;
}

// --------------------------- 중복 데이터 체크 함수 ---------------------------
// 주어진 comparator 기준으로 중복 키가 존재하는지 검사한다.
// 내부적으로 삽입정렬을 사용해서 정렬 후 인접 원소를 비교한다.

int has_duplicate(Student* original, int n, CompareFunc cmp) {
    Student* tmp = (Student*)malloc(sizeof(Student) * n);
    if (!tmp) return 0;
    copy_students(original, tmp, n);

    long long dummy_comp = 0;

    // 단순 삽입 정렬 사용
    for (int i = 1; i < n; i++) {
        Student key = tmp[i];
        int j = i - 1;
        while (j >= 0 && cmp(&tmp[j], &key, &dummy_comp) > 0) {
            tmp[j + 1] = tmp[j];
            j--;
        }
        tmp[j + 1] = key;
    }

    // 인접한 두 원소의 key가 같은지 검사
    for (int i = 0; i < n - 1; i++) {
        long long c = 0;
        if (cmp(&tmp[i], &tmp[i + 1], &c) == 0) {
            free(tmp);
            return 1; // 중복 존재
        }
    }

    free(tmp);
    return 0;
}

// --------------------------- 정렬 알고리즘 구현부 ---------------------------

// 정렬 함수 포인터 타입
typedef void (*SortFunc)(Student*, int, CompareFunc, long long*, long long*);

// 1. 버블 정렬 (stable)
void bubble_sort(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    (void)mem_usage;  // 추가 동적 메모리 사용 없음
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (cmp(&arr[j], &arr[j + 1], comp_cnt) > 0) {
                Student tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

// 2. 선택 정렬 (unstable)
void selection_sort(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    (void)mem_usage;
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (cmp(&arr[j], &arr[min_idx], comp_cnt) < 0) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            Student tmp = arr[i];
            arr[i] = arr[min_idx];
            arr[min_idx] = tmp;
        }
    }
}

// 3. 삽입 정렬 (stable)
void insertion_sort(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    (void)mem_usage;
    for (int i = 1; i < n; i++) {
        Student key = arr[i];
        int j = i - 1;
        while (j >= 0 && cmp(&arr[j], &key, comp_cnt) > 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// 4. 쉘 정렬 (기본: n/2, n/4, ..., 1) (unstable)
void shell_sort_basic(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    (void)mem_usage;
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            Student temp = arr[i];
            int j = i;
            while (j >= gap && cmp(&arr[j - gap], &temp, comp_cnt) > 0) {
                arr[j] = arr[j - gap];
                j -= gap;
            }
            arr[j] = temp;
        }
    }
}

// 4-B. 개선된 쉘 정렬 (Ciura 간격) - 과제 B
void shell_sort_optimized(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    (void)mem_usage;

    int gaps[] = { 1, 4, 10, 23, 57, 132, 301, 701, 1750, 3937, 8858 };
    int num_gaps = sizeof(gaps) / sizeof(gaps[0]);

    for (int g = num_gaps - 1; g >= 0; g--) {
        int gap = gaps[g];
        if (gap > n) continue;

        for (int i = gap; i < n; i++) {
            Student temp = arr[i];
            int j = i;
            while (j >= gap && cmp(&arr[j - gap], &temp, comp_cnt) > 0) {
                arr[j] = arr[j - gap];
                j -= gap;
            }
            arr[j] = temp;
        }
    }
}

// 5. 퀵 정렬 (기본: 마지막 원소 피봇) (unstable)
int partition_basic(Student* arr, int low, int high, CompareFunc cmp, long long* comp_cnt) {
    Student pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (cmp(&arr[j], &pivot, comp_cnt) <= 0) {
            i++;
            Student tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }
    }
    Student tmp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = tmp;
    return i + 1;
}

void quick_sort_rec_basic(Student* arr, int low, int high, CompareFunc cmp, long long* comp_cnt) {
    if (low < high) {
        int pi = partition_basic(arr, low, high, cmp, comp_cnt);
        quick_sort_rec_basic(arr, low, pi - 1, cmp, comp_cnt);
        quick_sort_rec_basic(arr, pi + 1, high, cmp, comp_cnt);
    }
}

void quick_sort_basic(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    (void)mem_usage;
    quick_sort_rec_basic(arr, 0, n - 1, cmp, comp_cnt);
}

// 5-B. 개선된 퀵 정렬 (median-of-three pivot) - 과제 B
int median_of_three(Student* arr, int low, int high, CompareFunc cmp, long long* comp_cnt) {
    int mid = (low + high) / 2;
    // 세 인덱스 중 중간값을 피봇으로 선택
    Student* a = &arr[low], * b = &arr[mid], * c = &arr[high];
    // a,b,c 를 비교하여 중간값의 인덱스를 반환
    long long dummy = 0;
    if (cmp(a, b, &dummy) > 0) {
        Student tmp = *a; *a = *b; *b = tmp;
    }
    if (cmp(a, c, &dummy) > 0) {
        Student tmp = *a; *a = *c; *c = tmp;
    }
    if (cmp(b, c, &dummy) > 0) {
        Student tmp = *b; *b = *c; *c = tmp;
    }
    // 이제 b가 중간값
    return mid;
}

int partition_optimized(Student* arr, int low, int high, CompareFunc cmp, long long* comp_cnt) {
    int m = median_of_three(arr, low, high, cmp, comp_cnt);
    // 피봇을 high 위치로 이동
    Student tmp = arr[m];
    arr[m] = arr[high];
    arr[high] = tmp;

    Student pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (cmp(&arr[j], &pivot, comp_cnt) <= 0) {
            i++;
            Student t = arr[i];
            arr[i] = arr[j];
            arr[j] = t;
        }
    }
    Student t2 = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = t2;
    return i + 1;
}

void quick_sort_rec_optimized(Student* arr, int low, int high, CompareFunc cmp, long long* comp_cnt) {
    if (low < high) {
        int pi = partition_optimized(arr, low, high, cmp, comp_cnt);
        quick_sort_rec_optimized(arr, low, pi - 1, cmp, comp_cnt);
        quick_sort_rec_optimized(arr, pi + 1, high, cmp, comp_cnt);
    }
}

void quick_sort_optimized(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    (void)mem_usage;
    quick_sort_rec_optimized(arr, 0, n - 1, cmp, comp_cnt);
}

// 6. 힙 정렬 (unstable)
void heapify(Student* arr, int n, int i, CompareFunc cmp, long long* comp_cnt) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && cmp(&arr[left], &arr[largest], comp_cnt) > 0) {
        largest = left;
    }
    if (right < n && cmp(&arr[right], &arr[largest], comp_cnt) > 0) {
        largest = right;
    }
    if (largest != i) {
        Student tmp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = tmp;
        heapify(arr, n, largest, cmp, comp_cnt);
    }
}

void heap_sort(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    (void)mem_usage;
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, cmp, comp_cnt);
    }
    for (int i = n - 1; i > 0; i--) {
        Student tmp = arr[0];
        arr[0] = arr[i];
        arr[i] = tmp;
        heapify(arr, i, 0, cmp, comp_cnt);
    }
}

// 7. 병합 정렬 (stable)
void merge(Student* arr, int left, int mid, int right, CompareFunc cmp,
    long long* comp_cnt, long long* mem_usage) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Student* L = (Student*)malloc(sizeof(Student) * n1);
    Student* R = (Student*)malloc(sizeof(Student) * n2);
    if (!L || !R) {
        fprintf(stderr, "malloc failed in merge\n");
        exit(1);
    }

    *mem_usage += sizeof(Student) * (n1 + n2);

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (cmp(&L[i], &R[j], comp_cnt) <= 0) {
            arr[k++] = L[i++];
        }
        else {
            arr[k++] = R[j++];
        }
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void merge_sort_rec(Student* arr, int left, int right, CompareFunc cmp,
    long long* comp_cnt, long long* mem_usage) {
    if (left < right) {
        int mid = (left + right) / 2;
        merge_sort_rec(arr, left, mid, cmp, comp_cnt, mem_usage);
        merge_sort_rec(arr, mid + 1, right, cmp, comp_cnt, mem_usage);
        merge(arr, left, mid, right, cmp, comp_cnt, mem_usage);
    }
}

void merge_sort(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    merge_sort_rec(arr, 0, n - 1, cmp, comp_cnt, mem_usage);
}

// 8. 기수 정렬 (Radix Sort) - ID(양의 정수) 기준, stable
int get_max_id(Student* arr, int n, long long* comp_cnt) {
    int max = arr[0].id;
    for (int i = 1; i < n; i++) {
        (*comp_cnt)++;
        if (arr[i].id > max) {
            max = arr[i].id;
        }
    }
    return max;
}

void counting_sort_by_digit(Student* arr, int n, int exp,
    long long* comp_cnt, long long* mem_usage) {
    Student* output = (Student*)malloc(sizeof(Student) * n);
    int count[10] = { 0 };

    if (!output) {
        fprintf(stderr, "malloc failed in radix\n");
        exit(1);
    }

    *mem_usage += sizeof(Student) * n;

    for (int i = 0; i < n; i++) {
        int digit = (arr[i].id / exp) % 10;
        count[digit]++;
    }
    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }
    for (int i = n - 1; i >= 0; i--) {
        int digit = (arr[i].id / exp) % 10;
        output[count[digit] - 1] = arr[i];
        count[digit]--;
    }
    for (int i = 0; i < n; i++) {
        (*comp_cnt)++; // 형식상 비교 카운트
        arr[i] = output[i];
    }
    free(output);
}

void radix_sort_id(Student* arr, int n, CompareFunc cmp_ignored,
    long long* comp_cnt, long long* mem_usage) {
    (void)cmp_ignored; // ID 기준 고정
    int max = get_max_id(arr, n, comp_cnt);
    for (int exp = 1; max / exp > 0; exp *= 10) {
        counting_sort_by_digit(arr, n, exp, comp_cnt, mem_usage);
    }
}

// 9-A. 트리 정렬 (BST 기반, unstable)
//     중복 키가 많으면 성능이 나빠질 수 있음 (과제 조건상 중복이면 생략 가능)
typedef struct TreeNode {
    Student data;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

TreeNode* bst_insert(TreeNode* root, Student value, CompareFunc cmp,
    long long* comp_cnt, long long* mem_usage) {
    if (root == NULL) {
        TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
        if (!node) {
            fprintf(stderr, "malloc failed in bst_insert\n");
            exit(1);
        }
        *mem_usage += sizeof(TreeNode);
        node->data = value;
        node->left = node->right = NULL;
        return node;
    }
    if (cmp(&value, &root->data, comp_cnt) < 0) {
        root->left = bst_insert(root->left, value, cmp, comp_cnt, mem_usage);
    }
    else {
        root->right = bst_insert(root->right, value, cmp, comp_cnt, mem_usage);
    }
    return root;
}

void bst_inorder(TreeNode* root, Student* arr, int* index) {
    if (!root) return;
    bst_inorder(root->left, arr, index);
    arr[(*index)++] = root->data;
    bst_inorder(root->right, arr, index);
}

void bst_free(TreeNode* root) {
    if (!root) return;
    bst_free(root->left);
    bst_free(root->right);
    free(root);
}

void tree_sort_bst(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    TreeNode* root = NULL;
    for (int i = 0; i < n; i++) {
        root = bst_insert(root, arr[i], cmp, comp_cnt, mem_usage);
    }
    int idx = 0;
    bst_inorder(root, arr, &idx);
    bst_free(root);
}

// 9-B. AVL Tree 정렬 (Tree 정렬 개선) - 과제 B
typedef struct AVLNode {
    Student data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

int avl_height(AVLNode* node) {
    if (!node) return 0;
    return node->height;
}

int avl_max(int a, int b) {
    return (a > b) ? a : b;
}

AVLNode* avl_new_node(Student data, long long* mem_usage) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) {
        fprintf(stderr, "malloc failed in avl_new_node\n");
        exit(1);
    }
    *mem_usage += sizeof(AVLNode);
    node->data = data;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

// 오른쪽 회전
AVLNode* avl_rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = avl_max(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = avl_max(avl_height(x->left), avl_height(x->right)) + 1;
    return x;
}

// 왼쪽 회전
AVLNode* avl_rotate_left(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = avl_max(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = avl_max(avl_height(y->left), avl_height(y->right)) + 1;
    return y;
}

int avl_get_balance(AVLNode* node) {
    if (!node) return 0;
    return avl_height(node->left) - avl_height(node->right);
}

AVLNode* avl_insert(AVLNode* node, Student data, CompareFunc cmp,
    long long* comp_cnt, long long* mem_usage) {
    if (!node) return avl_new_node(data, mem_usage);

    if (cmp(&data, &node->data, comp_cnt) < 0) {
        node->left = avl_insert(node->left, data, cmp, comp_cnt, mem_usage);
    }
    else {
        node->right = avl_insert(node->right, data, cmp, comp_cnt, mem_usage);
    }

    node->height = 1 + avl_max(avl_height(node->left), avl_height(node->right));
    int balance = avl_get_balance(node);

    // LL
    if (balance > 1 && cmp(&data, &node->left->data, comp_cnt) < 0) {
        return avl_rotate_right(node);
    }
    // RR
    if (balance < -1 && cmp(&data, &node->right->data, comp_cnt) > 0) {
        return avl_rotate_left(node);
    }
    // LR
    if (balance > 1 && cmp(&data, &node->left->data, comp_cnt) > 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    // RL
    if (balance < -1 && cmp(&data, &node->right->data, comp_cnt) < 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }
    return node;
}

void avl_inorder(AVLNode* root, Student* arr, int* index) {
    if (!root) return;
    avl_inorder(root->left, arr, index);
    arr[(*index)++] = root->data;
    avl_inorder(root->right, arr, index);
}

void avl_free(AVLNode* root) {
    if (!root) return;
    avl_free(root->left);
    avl_free(root->right);
    free(root);
}

void tree_sort_avl(Student* arr, int n, CompareFunc cmp, long long* comp_cnt, long long* mem_usage) {
    AVLNode* root = NULL;
    for (int i = 0; i < n; i++) {
        root = avl_insert(root, arr[i], cmp, comp_cnt, mem_usage);
    }
    int idx = 0;
    avl_inorder(root, arr, &idx);
    avl_free(root);
}

// --------------------------- 정렬 알고리즘 메타 정보 ---------------------------

typedef struct {
    const char* name;
    SortFunc func;
    int stable;         // stable 정렬인지 여부
    int is_heap_or_tree;// 힙/트리 정렬 여부 (중복 데이터일 때 생략 조건에 사용)
    int is_radix;       // 기수 정렬 여부 (ID 기준 전용)
} SortAlgo;

// A파트용 기본 9개 정렬 알고리즘
SortAlgo sort_algos_A[] = {
    {"Bubble",   bubble_sort,        1, 0, 0},
    {"Selection",selection_sort,     0, 0, 0},
    {"Insertion",insertion_sort,     1, 0, 0},
    {"ShellBasic",shell_sort_basic,  0, 0, 0},
    {"QuickBasic",quick_sort_basic,  0, 0, 0},
    {"Heap",     heap_sort,          0, 1, 0},
    {"Merge",    merge_sort,         1, 0, 0},
    {"RadixID",  radix_sort_id,      1, 0, 1},
    {"TreeBST",  tree_sort_bst,      0, 1, 0}
};
int NUM_SORT_A = sizeof(sort_algos_A) / sizeof(sort_algos_A[0]);

// B파트용 개선 알고리즘 (Shell, Quick, Tree)
SortAlgo sort_algos_B[] = {
    {"ShellOptim", shell_sort_optimized, 0, 0, 0},
    {"QuickOptim", quick_sort_optimized, 0, 0, 0},
    {"TreeAVL",    tree_sort_avl,        0, 1, 0}
};
int NUM_SORT_B = sizeof(sort_algos_B) / sizeof(sort_algos_B[0]);

// --------------------------- 과제 실행 헬퍼 함수 ---------------------------

// 한 기준에 대해 모든 정렬 알고리즘을 1000회 실행하고 평균 출력
void run_assignment_A_for_criterion(
    const char* criterion_name,
    Student* original, int n,
    CompareFunc cmp_asc,
    CompareFunc cmp_desc,
    int stable_only_for_gender,
    int is_gender_criterion
) {
    Student* arr = (Student*)malloc(sizeof(Student) * n);
    if (!arr) {
        fprintf(stderr, "malloc failed in run_assignment_A_for_criterion\n");
        return;
    }

    // 중복 데이터 존재 여부 (ID, NAME, GRADE 기준 등에서 힙/트리 생략 조건에 사용 가능)
    int has_dup_asc = has_duplicate(original, n, cmp_asc);

    printf("===== [과제 A] 정렬 기준: %s =====\n", criterion_name);
    printf("중복 데이터(오름차순 기준) 존재 여부: %s\n", has_dup_asc ? "예" : "아니오");
    printf("각 정렬 알고리즘 %d회 실행 후 평균 비교 횟수 / 평균 추가 메모리\n\n", RUNS);

    for (int ai = 0; ai < NUM_SORT_A; ai++) {
        SortAlgo algo = sort_algos_A[ai];

        // GENDER 기준일 때는 stable 정렬만 사용 (문제 조건)
        if (is_gender_criterion && stable_only_for_gender && !algo.stable) {
            continue;
        }

        // 중복 데이터가 있을 경우 힙/트리 정렬은 수행하지 않음 (문제 조건)
        if (has_dup_asc && algo.is_heap_or_tree) {
            printf("[%s][%s] 중복 데이터로 인해 실행하지 않음 (힙/트리 정렬 제외)\n\n",
                algo.name, criterion_name);
            continue;
        }

        // 기수 정렬은 ID 기준에서만 의미 있음
        if (algo.is_radix && strcmp(criterion_name, "ID") != 0) {
            continue;
        }

        // 오름차순 기준
        long long total_comp = 0, total_mem = 0;
        for (int r = 0; r < RUNS; r++) {
            copy_students(original, arr, n);
            long long comp = 0, mem = 0;
            algo.func(arr, n, cmp_asc, &comp, &mem);
            total_comp += comp;
            total_mem += mem;
        }
        printf("[%s][%s 오름차순] 평균 비교 횟수: %.2f, 평균 추가 메모리: %.2f bytes\n",
            algo.name, criterion_name,
            (double)total_comp / RUNS,
            (double)total_mem / RUNS);

        // 내림차순 기준 (기수 정렬은 생략)
        if (!algo.is_radix) {
            total_comp = total_mem = 0;
            for (int r = 0; r < RUNS; r++) {
                copy_students(original, arr, n);
                long long comp = 0, mem = 0;
                algo.func(arr, n, cmp_desc, &comp, &mem);
                total_comp += comp;
                total_mem += mem;
            }
            printf("[%s][%s 내림차순] 평균 비교 횟수: %.2f, 평균 추가 메모리: %.2f bytes\n",
                algo.name, criterion_name,
                (double)total_comp / RUNS,
                (double)total_mem / RUNS);
        }

        printf("\n");
    }

    free(arr);
}

// 과제 B 실행: 개선된 Shell, Quick, Tree(AVL)를 각 기준으로 테스트 가능
void run_assignment_B_for_criterion(
    const char* criterion_name,
    Student* original, int n,
    CompareFunc cmp
) {
    Student* arr = (Student*)malloc(sizeof(Student) * n);
    if (!arr) {
        fprintf(stderr, "malloc failed in run_assignment_B_for_criterion\n");
        return;
    }

    printf("===== [과제 B] 정렬 기준: %s =====\n", criterion_name);
    printf("각 개선 정렬 알고리즘 %d회 실행 후 평균 비교 횟수 / 평균 추가 메모리\n\n", RUNS);

    int has_dup = has_duplicate(original, n, cmp);

    for (int ai = 0; ai < NUM_SORT_B; ai++) {
        SortAlgo algo = sort_algos_B[ai];

        if (has_dup && algo.is_heap_or_tree) {
            printf("[%s][%s] 중복 데이터로 인해 실행하지 않음 (트리 정렬)\n\n",
                algo.name, criterion_name);
            continue;
        }

        long long total_comp = 0, total_mem = 0;
        for (int r = 0; r < RUNS; r++) {
            copy_students(original, arr, n);
            long long comp = 0, mem = 0;
            algo.func(arr, n, cmp, &comp, &mem);
            total_comp += comp;
            total_mem += mem;
        }
        printf("[%s][%s] 평균 비교 횟수: %.2f, 평균 추가 메모리: %.2f bytes\n\n",
            algo.name, criterion_name,
            (double)total_comp / RUNS,
            (double)total_mem / RUNS);
    }

    free(arr);
}

// --------------------------- 메인 함수 ---------------------------

int main(void) {
    int count = 0;

    // CSV 파일에서 학생 데이터 로드 (파일명은 필요에 따라 수정)
    Student* original = load_students("dataset_id_ascending.csv", &count);
    if (!original) {
        fprintf(stderr, "학생 데이터 로드 실패\n");
        return 1;
    }

    printf("학생 수: %d\n\n", count);

    // ---------------- 과제 A: 9개 정렬 알고리즘 ----------------

    // 1) ID 기준
    run_assignment_A_for_criterion("ID", original, count,
        cmp_id_asc, cmp_id_desc,
        0, 0);

    // 2) NAME 기준
    run_assignment_A_for_criterion("NAME", original, count,
        cmp_name_asc, cmp_name_desc,
        0, 0);

    // 3) GENDER 기준 (Stable 정렬만 사용)
    run_assignment_A_for_criterion("GENDER", original, count,
        cmp_gender_asc, cmp_gender_desc,
        1, 1); // stable_only_for_gender = 1, is_gender_criterion = 1

    // 4) GRADE 기준 (총점 + tie-break)
    run_assignment_A_for_criterion("GRADE", original, count,
        cmp_grade_asc, cmp_grade_desc,
        0, 0);

    // ---------------- 과제 B: Shell / Quick / Tree(AVL) 개선 ----------------

    printf("\n\n====================== 과제 B: 정렬 알고리즘 개선 ======================\n\n");

    // 예시로 ID 기준으로 개선 알고리즘 테스트
    run_assignment_B_for_criterion("ID", original, count, cmp_id_asc);

    // 원한다면 NAME / GRADE 기준으로도 추가 실행 가능:
    // run_assignment_B_for_criterion("NAME", original, count, cmp_name_asc);
    // run_assignment_B_for_criterion("GRADE", original, count, cmp_grade_asc);

    free(original);
    return 0;
}
