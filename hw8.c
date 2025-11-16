#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000      // 배열 크기
#define RUNS 100     // 반복 실행 횟수
#define MAX_VALUE 1000000  // 데이터 값 범위 0 ~ 1,000,000

// --------------------------- 유틸리티 함수 ---------------------------

// 무작위 배열 생성 함수
void generate_random_array(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        // 0 ~ MAX_VALUE 사이의 무작위 정수 생성
        arr[i] = rand() % (MAX_VALUE + 1);
    }
}

// 배열 복사 함수
void copy_array(int* src, int* dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

// --------------------------- 정렬 함수들 ---------------------------

// 단순 삽입 정렬
// comp_count: 비교 횟수를 누적해서 저장할 포인터
void insertion_sort(int* arr, int n, long long* comp_count) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;

        // arr[j] > key 비교가 일어날 때마다 comp_count 증가
        while (j >= 0) {
            (*comp_count)++;         // arr[j] > key 비교 1회 수행
            if (arr[j] > key) {
                arr[j + 1] = arr[j];
                j--;
            }
            else {
                // 더 이상 key보다 큰 원소가 없으면 종료
                break;
            }
        }
        arr[j + 1] = key;
    }
}

// 기본 쉘 정렬 (간격: n/2, n/4, ..., 1)
void shell_sort_basic(int* arr, int n, long long* comp_count) {
    // gap을 n/2부터 시작해서 1까지 1/2씩 줄임
    for (int gap = n / 2; gap > 0; gap /= 2) {
        // gap 간격 삽입 정렬 수행
        for (int i = gap; i < n; i++) {
            int temp = arr[i];
            int j = i;

            while (j >= gap) {
                (*comp_count)++;  // arr[j - gap] > temp 비교 1회
                if (arr[j - gap] > temp) {
                    arr[j] = arr[j - gap];
                    j -= gap;
                }
                else {
                    break;
                }
            }
            arr[j] = temp;
        }
    }
}

// 개선된 쉘 정렬 (Ciura 간격 시퀀스 사용)
// Ciura gap sequence는 실험적으로 좋은 성능(비교 횟수 감소)을 보이는 것으로 유명함
void shell_sort_optimized(int* arr, int n, long long* comp_count) {
    // Ciura가 제안한 간격 시퀀스 (필요 시 10,000까지 확장)
    int gaps[] = { 1, 4, 10, 23, 57, 132, 301, 701, 1750, 3937, 8858, 19821 };
    int num_gaps = sizeof(gaps) / sizeof(gaps[0]);

    // 가장 큰 gap부터 시작해서 1까지 내려가며 사용
    for (int g = num_gaps - 1; g >= 0; g--) {
        int gap = gaps[g];
        if (gap > n) continue;  // 배열 크기보다 큰 gap은 건너뜀

        for (int i = gap; i < n; i++) {
            int temp = arr[i];
            int j = i;

            while (j >= gap) {
                (*comp_count)++;  // arr[j - gap] > temp 비교 1회
                if (arr[j - gap] > temp) {
                    arr[j] = arr[j - gap];
                    j -= gap;
                }
                else {
                    break;
                }
            }
            arr[j] = temp;
        }
    }
}

// --------------------------- 메인 함수 ---------------------------

int main(void) {
    // 무작위성 초기화 (현재 시간 기반 seed)
    srand((unsigned int)time(NULL));

    int original[N];
    int arr_insertion[N];
    int arr_shell_basic[N];
    int arr_shell_opt[N];

    long long total_comp_insertion = 0;   // 삽입 정렬 전체 비교 횟수
    long long total_comp_shell_basic = 0; // 기본 쉘 정렬 전체 비교 횟수
    long long total_comp_shell_opt = 0;   // 개선 쉘 정렬 전체 비교 횟수

    for (int run = 0; run < RUNS; run++) {
        // 1) 원본 무작위 배열 생성
        generate_random_array(original, N);

        // 2) 알고리즘별로 동일한 데이터 사용을 위해 복사
        copy_array(original, arr_insertion, N);
        copy_array(original, arr_shell_basic, N);
        copy_array(original, arr_shell_opt, N);

        // 3) 각 정렬 수행 및 비교 횟수 측정
        long long comp_ins = 0;
        long long comp_shell_b = 0;
        long long comp_shell_o = 0;

        insertion_sort(arr_insertion, N, &comp_ins);
        shell_sort_basic(arr_shell_basic, N, &comp_shell_b);
        shell_sort_optimized(arr_shell_opt, N, &comp_shell_o);

        // 4) 각 정렬의 비교 횟수를 전체 합에 누적
        total_comp_insertion += comp_ins;
        total_comp_shell_basic += comp_shell_b;
        total_comp_shell_opt += comp_shell_o;
    }

    // 평균 비교 횟수 계산 (double로 나누기)
    double avg_comp_insertion = (double)total_comp_insertion / RUNS;
    double avg_comp_shell_basic = (double)total_comp_shell_basic / RUNS;
    double avg_comp_shell_opt = (double)total_comp_shell_opt / RUNS;

    // --------------------------- 결과 출력 ---------------------------

    printf("데이터 크기: %d, 실행 횟수: %d\n", N, RUNS);
    printf("값 범위: 0 ~ %d\n\n", MAX_VALUE);

    printf("단순 삽입 정렬 평균 비교 횟수       : %.2f\n", avg_comp_insertion);
    printf("기본 쉘 정렬 (n/2, n/4, ...) 평균 비교 횟수: %.2f\n", avg_comp_shell_basic);
    printf("개선된 쉘 정렬 (Ciura 간격) 평균 비교 횟수 : %.2f\n", avg_comp_shell_opt);

    return 0;
}
