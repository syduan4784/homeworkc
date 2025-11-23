#define _CRT_SECURE_NO_WARNINGS   // Visual Studio에서 안전하지 않은 함수 경고 제거

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 200

// --------------------------- 학생 구조체 정의 ---------------------------
// 과제 09에서 사용한 학생 구조체에
// "세 가지 성적의 곱"을 저장하는 필드를 추가한다.

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char gender;   // 'M', 'F' 등
    int korean;
    int english;
    int math;
    int grade_product;   // 국어 * 영어 * 수학 (세 가지 성적의 곱)
} Student;


// --------------------------- 학생 데이터 로드 함수 ---------------------------
// CSV 파일에서 학생 정보를 읽어서 동적 배열로 반환한다.
// 파일 형식 예:
// id,name,gender,korean,english,math
// 1,Kim,M,80,90,85
// ...

Student* load_students(const char* filename, int* out_count) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }

    char line[MAX_LINE_LEN];
    int capacity = 10;
    int count = 0;
    Student* arr = (Student*)malloc(sizeof(Student) * capacity);
    if (!arr) {
        perror("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    // 첫 줄 헤더 스킵
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        free(arr);
        *out_count = 0;
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (count >= capacity) {
            capacity *= 2;
            Student* temp = (Student*)realloc(arr, sizeof(Student) * capacity);
            if (!temp) {
                perror("Reallocation failed");
                free(arr);
                fclose(fp);
                return NULL;
            }
            arr = temp;
        }

        Student s;
        char* token;

        // id
        token = strtok(line, ",");
        if (!token) continue;
        s.id = atoi(token);

        // name
        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(s.name, token, MAX_NAME_LEN);
        s.name[MAX_NAME_LEN - 1] = '\0';

        // gender
        token = strtok(NULL, ",");
        if (!token) continue;
        s.gender = token[0];

        // korean
        token = strtok(NULL, ",");
        if (!token) continue;
        s.korean = atoi(token);

        // english
        token = strtok(NULL, ",");
        if (!token) continue;
        s.english = atoi(token);

        // math
        token = strtok(NULL, ",");
        if (!token) continue;
        s.math = atoi(token);

        // ★ 세 가지 성적의 곱을 추가로 계산하여 저장
        s.grade_product = s.korean * s.english * s.math;

        arr[count++] = s;
    }

    fclose(fp);

    Student* tight = (Student*)realloc(arr, sizeof(Student) * count);
    if (!tight) {
        fprintf(stderr, "Warning: Tight reallocation failed, using original memory.\n");
        *out_count = count;
        return arr;
    }

    *out_count = count;
    return tight;
}


// --------------------------- 순차 탐색 (Linear Search) ---------------------------
// 배열에서 target과 같은 grade_product를 찾을 때까지
// 한 칸씩 검사하면서 비교 횟수를 센다.

long long linear_search_grade_product(Student* arr, int n, int target) {
    long long comp_count = 0;

    for (int i = 0; i < n; i++) {
        comp_count++;   // grade_product == target 비교 1회
        if (arr[i].grade_product == target) {
            // 찾았다고 해서 바로 종료해도 되고
            // 끝까지 탐색해도 되지만, 여기서는 일반적인 방식으로 즉시 종료
            return comp_count;
        }
    }

    // 찾지 못한 경우에도 지금까지의 비교 횟수를 반환
    return comp_count;
}


// --------------------------- 퀵 정렬 (grade_product 기준) ---------------------------

// 정렬 과정에서의 비교 횟수를 기록하기 위한 전역 변수 대신
// 포인터 인자로 전달한다.
int cmp_grade_product(const Student* a, const Student* b, long long* comp_cnt) {
    (*comp_cnt)++;  // 비교 1회

    if (a->grade_product < b->grade_product) return -1;
    if (a->grade_product > b->grade_product) return 1;
    return 0;
}

// partition 함수: 마지막 원소를 피봇으로 사용하는 기본 버전
int partition_quick(Student* arr, int low, int high, long long* comp_cnt) {
    Student pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (cmp_grade_product(&arr[j], &pivot, comp_cnt) <= 0) {
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

void quick_sort_grade_product_rec(Student* arr, int low, int high, long long* comp_cnt) {
    if (low < high) {
        int pi = partition_quick(arr, low, high, comp_cnt);
        quick_sort_grade_product_rec(arr, low, pi - 1, comp_cnt);
        quick_sort_grade_product_rec(arr, pi + 1, high, comp_cnt);
    }
}

long long quick_sort_grade_product(Student* arr, int n) {
    long long comp_cnt = 0;
    quick_sort_grade_product_rec(arr, 0, n - 1, &comp_cnt);
    return comp_cnt;
}


// --------------------------- 이진 탐색 (Binary Search) ---------------------------
// grade_product가 오름차순으로 정렬되어 있다는 가정 하에
// target을 찾기 위한 이진 탐색을 수행하고 비교 횟수를 센다.

long long binary_search_grade_product(Student* arr, int n, int target) {
    int low = 0;
    int high = n - 1;
    long long comp_cnt = 0;

    while (low <= high) {
        int mid = (low + high) / 2;

        comp_cnt++;  // arr[mid].grade_product == target 비교 1회
        if (arr[mid].grade_product == target) {
            return comp_cnt;
        }

        comp_cnt++;  // arr[mid].grade_product < target 비교 1회
        if (arr[mid].grade_product < target) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }

    // 찾지 못한 경우에도 지금까지의 비교 횟수를 반환
    return comp_cnt;
}


// --------------------------- 메인 함수 ---------------------------

int main(void) {
    int count = 0;

    // 과제 09에서 사용한 데이터셋 파일 이름 (필요에 따라 수정)
    const char* filename = "dataset_id_ascending.csv";

    // 1) 학생 데이터 로드 + 세 가지 성적의 곱 계산
    Student* students = load_students(filename, &count);
    if (!students) {
        fprintf(stderr, "학생 데이터 로드 실패\n");
        return 1;
    }

    printf("학생 수: %d명\n", count);

    // (확인용) 처음 몇 명의 grade_product를 출력해 볼 수도 있다.
    // for (int i = 0; i < (count < 5 ? count : 5); i++) {
    //     printf("ID=%d, 이름=%s, 국어=%d, 영어=%d, 수학=%d, 곱=%d\n",
    //            students[i].id, students[i].name,
    //            students[i].korean, students[i].english, students[i].math,
    //            students[i].grade_product);
    // }

    // 2) 0 ~ 1,000,000 사이의 임의의 값 선택
    srand((unsigned int)time(NULL));
    int target = rand() % 1000001;  // 0 ~ 1,000,000

    printf("\n임의로 선택된 값 (0 ~ 1,000,000): %d\n\n", target);

    // 3) 순차 탐색 수행 및 비교 횟수 측정
    long long seq_comp = linear_search_grade_product(students, count, target);
    printf("[순차 탐색] 비교 횟수: %lld\n", seq_comp);

    // 4) 같은 값에 대해, 정렬 후 이진 탐색 수행
    //    - 정렬 방법은 자유 → 여기서는 grade_product 기준 퀵 정렬 사용
    //    - 정렬 과정 비교 횟수 + 이진 탐색 비교 횟수를 합산

    // 정렬을 위해 배열을 복사 (원본 보존을 위해)
    Student* sorted = (Student*)malloc(sizeof(Student) * count);
    if (!sorted) {
        fprintf(stderr, "메모리 할당 실패\n");
        free(students);
        return 1;
    }
    for (int i = 0; i < count; i++) {
        sorted[i] = students[i];
    }

    // 퀵 정렬 수행 및 비교 횟수
    long long sort_comp = quick_sort_grade_product(sorted, count);

    // 이진 탐색 수행 및 비교 횟수
    long long bin_comp = binary_search_grade_product(sorted, count, target);

    long long total_comp = sort_comp + bin_comp;

    printf("\n[퀵 정렬 + 이진 탐색]\n");
    printf("  정렬(퀵 정렬) 비교 횟수      : %lld\n", sort_comp);
    printf("  이진 탐색 비교 횟수          : %lld\n", bin_comp);
    printf("  정렬 + 이진 탐색 총 비교 횟수: %lld\n", total_comp);

    // 정리
    free(sorted);
    free(students);

    return 0;
}
