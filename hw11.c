#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 200

// --------------------------- 학생 구조체 정의 ---------------------------
// 과제 09에서 사용한 학생 정보 구조체와 동일하게 사용한다.

typedef struct {
    int id;                         // 학번 (키 값)
    char name[MAX_NAME_LEN];        // 이름
    char gender;                    // 성별 ('M', 'F' 등)
    int korean;                     // 국어 점수
    int english;                    // 영어 점수
    int math;                       // 수학 점수
} Student;

// --------------------------- CSV 파일 로드 함수 ---------------------------
// CSV 파일(과제 09 데이터셋)을 읽어서 Student 배열로 만든다.
// 파일 첫 줄은 헤더라고 가정하고 스킵한다.

Student* load_students(const char* filename, int* out_count) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("파일 열기 실패");
        return NULL;
    }

    char line[MAX_LINE_LEN];
    int capacity = 100;
    int count = 0;
    Student* arr = (Student*)malloc(sizeof(Student) * capacity);
    if (!arr) {
        perror("메모리 할당 실패");
        fclose(fp);
        return NULL;
    }

    // 첫 줄 헤더 스킵
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        if (count >= capacity) {
            capacity *= 2;
            Student* temp = (Student*)realloc(arr, sizeof(Student) * capacity);
            if (!temp) {
                perror("realloc 실패");
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

    Student* tight = (Student*)realloc(arr, sizeof(Student) * count);
    if (!tight) {
        // realloc 실패 시, 기존 메모리 그냥 사용
        *out_count = count;
        return arr;
    }

    *out_count = count;
    return tight;
}

// --------------------------- 유틸리티 함수 ---------------------------

void copy_students(Student* src, Student* dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

// 디버깅용: 학생 1명 정보 출력
void print_student(const Student* s) {
    printf("ID=%d, Name=%s, Gender=%c, K=%d, E=%d, M=%d\n",
        s->id, s->name, s->gender, s->korean, s->english, s->math);
}

// --------------------------- 비정렬 배열(unsorted array) 연산 ---------------------------
// 검색: 순차 탐색 (Linear Search)
// 삽입: 배열 끝에 추가 (중복 검사 없음)
// 삭제: 순차 탐색으로 위치 찾은 후, 마지막 원소를 가져와서 덮어쓰기

int linear_search_unsorted(Student* arr, int n, int key, long long* cmp_count) {
    *cmp_count = 0;
    for (int i = 0; i < n; i++) {
        (*cmp_count)++;                   // arr[i].id == key 비교 1회
        if (arr[i].id == key) {
            return i;                     // 찾았을 때 인덱스 반환
        }
    }
    return -1;                            // 못 찾으면 -1
}

int insert_unsorted(Student* arr, int* n, int capacity, Student x, long long* cmp_count) {
    // 비정렬 배열 삽입은 단순히 끝에 추가 → 비교 연산이 필요 없음
    *cmp_count = 0;                       // 비교 횟수 0
    if (*n >= capacity) {
        // 실제 과제에서는 capacity 체크를 할 수도 있지만, 여기서는 단순히 실패 처리
        return 0;
    }
    arr[*n] = x;
    (*n)++;
    return 1;
}

int delete_unsorted(Student* arr, int* n, int key, long long* cmp_count) {
    // 순차 탐색으로 key 위치를 찾고, 마지막 원소를 가져와 덮어쓴다.
    long long search_cmp = 0;
    int pos = linear_search_unsorted(arr, *n, key, &search_cmp);
    *cmp_count = search_cmp;              // 삭제의 비교횟수 = 탐색 비교횟수로 정의

    if (pos == -1) {
        // 찾지 못한 경우
        return 0;
    }

    arr[pos] = arr[*n - 1];               // 마지막 학생을 해당 위치로 이동
    (*n)--;                               // 크기 감소
    return 1;
}

// --------------------------- 정렬 배열(sorted array) 연산 ---------------------------
// 검색: 이진 탐색(Binary Search)
// 삽입: 이진 탐색으로 삽입 위치 찾은 후, 오른쪽으로 shift
// 삭제: 이진 탐색으로 위치 찾은 후, 왼쪽으로 shift

int binary_search_sorted(Student* arr, int n, int key, long long* cmp_count) {
    int left = 0, right = n - 1;
    *cmp_count = 0;

    while (left <= right) {
        int mid = (left + right) / 2;
        (*cmp_count)++;                   // key == arr[mid].id 비교
        if (key == arr[mid].id) {
            return mid;
        }
        (*cmp_count)++;                   // key < arr[mid].id 비교
        if (key < arr[mid].id) {
            right = mid - 1;
        }
        else {
            // 여기서는 key > arr[mid].id 라는 의미
            left = mid + 1;
        }
    }
    return -1;
}

// 삽입 위치를 찾는 이진 탐색 (lower bound 비슷한 동작)
// 반환값: key를 삽입해야 할 인덱스 (0 ~ n)
int find_insert_pos_sorted(Student* arr, int n, int key, long long* cmp_count) {
    int left = 0, right = n - 1;
    int pos = n;                          // 기본값: 맨 뒤에 삽입
    *cmp_count = 0;

    while (left <= right) {
        int mid = (left + right) / 2;
        (*cmp_count)++;                   // key <= arr[mid].id 비교
        if (key <= arr[mid].id) {
            pos = mid;
            right = mid - 1;
        }
        else {
            left = mid + 1;
        }
    }
    return pos;
}

int insert_sorted(Student* arr, int* n, int capacity, Student x, long long* cmp_count) {
    if (*n >= capacity) return 0;

    long long c = 0;
    int pos = find_insert_pos_sorted(arr, *n, x.id, &c);
    *cmp_count = c;                       // 삽입에서의 비교 횟수 = 위치 탐색 비교 횟수

    // 이미 같은 id가 존재하면, 여기서는 "삽입하지 않고" 넘어간다.
    if (pos < *n) {
        (*cmp_count)++;                   // arr[pos].id == x.id 비교
        if (arr[pos].id == x.id) {
            // 중복 ID → 삽입 실패로 처리
            return 0;
        }
    }

    // 오른쪽으로 한 칸씩 밀기
    for (int i = *n; i > pos; i--) {
        arr[i] = arr[i - 1];
    }
    arr[pos] = x;
    (*n)++;
    return 1;
}

int delete_sorted(Student* arr, int* n, int key, long long* cmp_count) {
    long long c = 0;
    int pos = binary_search_sorted(arr, *n, key, &c);
    *cmp_count = c;                       // 삭제의 비교 횟수 = 이진 탐색 비교 횟수

    if (pos == -1) {
        // 못 찾음
        return 0;
    }
    // 왼쪽으로 shift
    for (int i = pos; i < *n - 1; i++) {
        arr[i] = arr[i + 1];
    }
    (*n)--;
    return 1;
}

// --------------------------- AVL 트리 구조 및 연산 ---------------------------
// 키: 학생의 id (int)
// 각 연산에서 id 비교가 일어날 때마다 cmp_count를 증가시킨다.

typedef struct AVLNode {
    Student data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

int avl_height(AVLNode* node) {
    return node ? node->height : 0;
}

int avl_max(int a, int b) {
    return (a > b) ? a : b;
}

// 우회전 (Right Rotation)
AVLNode* avl_rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = avl_max(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = avl_max(avl_height(x->left), avl_height(x->right)) + 1;

    return x;
}

// 좌회전 (Left Rotation)
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

AVLNode* avl_new_node(Student data) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) {
        fprintf(stderr, "AVL 노드 malloc 실패\n");
        exit(1);
    }
    node->data = data;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

// AVL 삽입
AVLNode* avl_insert(AVLNode* node, Student data, long long* cmp_count) {
    if (!node) {
        return avl_new_node(data);
    }

    (*cmp_count)++;                          // data.id < node->data.id 비교
    if (data.id < node->data.id) {
        node->left = avl_insert(node->left, data, cmp_count);
    }
    else {
        (*cmp_count)++;                      // data.id > node->data.id 비교
        if (data.id > node->data.id) {
            node->right = avl_insert(node->right, data, cmp_count);
        }
        else {
            // data.id == node->data.id → 중복 키: 여기서는 그냥 무시(삽입 안 함)
            return node;
        }
    }

    // 높이 갱신
    node->height = 1 + avl_max(avl_height(node->left), avl_height(node->right));

    // 균형 인수 계산
    int balance = avl_get_balance(node);

    // LL 케이스
    if (balance > 1 && data.id < node->left->data.id) {
        return avl_rotate_right(node);
    }
    // RR 케이스
    if (balance < -1 && data.id > node->right->data.id) {
        return avl_rotate_left(node);
    }
    // LR 케이스
    if (balance > 1 && data.id > node->left->data.id) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    // RL 케이스
    if (balance < -1 && data.id < node->right->data.id) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }

    return node;
}

// AVL 탐색
AVLNode* avl_search(AVLNode* root, int key, long long* cmp_count) {
    if (!root) return NULL;

    (*cmp_count)++;                           // key == root->data.id 비교
    if (key == root->data.id) {
        return root;
    }

    (*cmp_count)++;                           // key < root->data.id 비교
    if (key < root->data.id) {
        return avl_search(root->left, key, cmp_count);
    }
    else {
        return avl_search(root->right, key, cmp_count);
    }
}

// 트리에서 최소 id를 가진 노드 찾기 (삭제 시 사용)
AVLNode* avl_min_value_node(AVLNode* node) {
    AVLNode* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// AVL 삭제
AVLNode* avl_delete(AVLNode* root, int key, long long* cmp_count) {
    if (!root) return NULL;

    (*cmp_count)++;                            // key < root->data.id 비교
    if (key < root->data.id) {
        root->left = avl_delete(root->left, key, cmp_count);
    }
    else {
        (*cmp_count)++;                        // key > root->data.id 비교
        if (key > root->data.id) {
            root->right = avl_delete(root->right, key, cmp_count);
        }
        else {
            // key == root->data.id → 이 노드를 삭제
            if (root->left == NULL || root->right == NULL) {
                AVLNode* temp = root->left ? root->left : root->right;
                if (!temp) {
                    // 자식이 없는 경우
                    temp = root;
                    root = NULL;
                }
                else {
                    // 자식이 1개인 경우
                    *root = *temp;
                }
                free(temp);
            }
            else {
                // 자식이 2개인 경우: 오른쪽 서브트리에서 최소값
                AVLNode* temp = avl_min_value_node(root->right);
                root->data = temp->data;
                root->right = avl_delete(root->right, temp->data.id, cmp_count);
            }
        }
    }

    if (!root) return root;

    // 높이 갱신
    root->height = 1 + avl_max(avl_height(root->left), avl_height(root->right));

    // 균형 인수 계산
    int balance = avl_get_balance(root);

    // 4가지 회전 케이스 처리

    // LL
    if (balance > 1 && avl_get_balance(root->left) >= 0) {
        return avl_rotate_right(root);
    }

    // LR
    if (balance > 1 && avl_get_balance(root->left) < 0) {
        root->left = avl_rotate_left(root->left);
        return avl_rotate_right(root);
    }

    // RR
    if (balance < -1 && avl_get_balance(root->right) <= 0) {
        return avl_rotate_left(root);
    }

    // RL
    if (balance < -1 && avl_get_balance(root->right) > 0) {
        root->right = avl_rotate_right(root->right);
        return avl_rotate_left(root);
    }

    return root;
}

void avl_free(AVLNode* root) {
    if (!root) return;
    avl_free(root->left);
    avl_free(root->right);
    free(root);
}

// --------------------------- 메인 함수 ---------------------------
// 과제 11 요구사항:
// 1) 비정렬 배열, 정렬 배열, AVL Tree 각각에 대해
//    - 삽입 / 삭제 / 검색을 수행
//    - 각 과정에서 비교 횟수를 출력
//    (비정렬 배열 검색: 순차 탐색, 정렬 배열 검색: 이진 탐색 사용)

int main(void) {
    int count = 0;

    // 데이터셋 파일 이름 (같은 폴더에 위치해야 함)
    // 필요시 파일 이름을 교수님이 준 CSV 이름으로 수정
    const char* filename = "dataset_id_ascending.csv";

    Student* original = load_students(filename, &count);
    if (!original) {
        fprintf(stderr, "학생 데이터 로드 실패\n");
        return 1;
    }

    printf("로드된 학생 수: %d명\n\n", count);

    // ----------------- 비정렬 배열 / 정렬 배열 준비 -----------------
    // capacity를 여유 있게 잡아서 삽입 연산도 가능하게 한다.
    int capacity = count + 1000;

    Student* unsorted = (Student*)malloc(sizeof(Student) * capacity);
    Student* sorted = (Student*)malloc(sizeof(Student) * capacity);
    if (!unsorted || !sorted) {
        fprintf(stderr, "배열 메모리 할당 실패\n");
        free(original);
        return 1;
    }

    // 비정렬 배열: original 순서를 그대로 사용 (여기서는 original 자체가 어떤 순서든 상관 없음)
    copy_students(original, unsorted, count);
    int n_unsorted = count;

    // 정렬 배열: 과제 09의 dataset_id_ascending.csv는 이미 ID 기준 오름차순이라고 가정
    // 만약 정렬이 안 되어 있다면, 여기서 별도의 정렬 알고리즘을 사용해서 정렬해도 된다.
    copy_students(original, sorted, count);
    int n_sorted = count;

    // ----------------- AVL 트리 초기 구성 -----------------
    AVLNode* avl_root = NULL;
    long long cmp_dummy = 0;
    for (int i = 0; i < count; i++) {
        avl_root = avl_insert(avl_root, original[i], &cmp_dummy);
    }

    // ----------------- 사용자 인터페이스 (입출력 포맷 자유) -----------------
    while (1) {
        int menu;
        printf("===== 과제 11: 배열 & AVL 비교 횟수 측정 =====\n");
        printf("1. ID 검색 (Search)\n");
        printf("2. ID 삽입 (Insert)\n");
        printf("3. ID 삭제 (Delete)\n");
        printf("0. 종료 (Exit)\n");
        printf("메뉴를 선택하세요: ");
        if (scanf("%d", &menu) != 1) break;

        if (menu == 0) {
            printf("프로그램을 종료합니다.\n");
            break;
        }

        if (menu == 1) {
            // ----------- 검색(Search) -----------
            int key;
            printf("검색할 학생 ID를 입력하세요: ");
            scanf("%d", &key);

            long long cmp_unsorted = 0, cmp_sorted = 0, cmp_avl = 0;

            int pos_unsorted = linear_search_unsorted(unsorted, n_unsorted, key, &cmp_unsorted);
            int pos_sorted = binary_search_sorted(sorted, n_sorted, key, &cmp_sorted);
            AVLNode* found = avl_search(avl_root, key, &cmp_avl);

            printf("\n[검색 결과]\n");
            printf("비정렬 배열 (순차 탐색): 비교 횟수 = %lld, 결과 = %s\n",
                cmp_unsorted, (pos_unsorted == -1 ? "없음" : "있음"));
            printf("정렬 배열 (이진 탐색):   비교 횟수 = %lld, 결과 = %s\n",
                cmp_sorted, (pos_sorted == -1 ? "없음" : "있음"));
            printf("AVL 트리 탐색:          비교 횟수 = %lld, 결과 = %s\n\n",
                cmp_avl, (found == NULL ? "없음" : "있음"));
        }
        else if (menu == 2) {
            // ----------- 삽입(Insert) -----------
            int key;
            printf("삽입할 학생 ID를 입력하세요: ");
            scanf("%d", &key);

            // 과제에서는 비교 횟수가 중요한 것이므로,
            // 새로 삽입하는 학생의 다른 필드는 간단히 채운다.
            Student s;
            s.id = key;
            strcpy(s.name, "NEW_STUDENT");
            s.gender = 'M';
            s.korean = s.english = s.math = 0;

            long long cmp_unsorted = 0, cmp_sorted = 0, cmp_avl = 0;

            int ok1 = insert_unsorted(unsorted, &n_unsorted, capacity, s, &cmp_unsorted);
            int ok2 = insert_sorted(sorted, &n_sorted, capacity, s, &cmp_sorted);
            avl_root = avl_insert(avl_root, s, &cmp_avl);

            printf("\n[삽입 결과] (ID=%d)\n", key);
            printf("비정렬 배열 삽입: 비교 횟수 = %lld, 결과 = %s\n",
                cmp_unsorted, (ok1 ? "성공" : "실패(공간 부족)"));
            printf("정렬 배열 삽입:   비교 횟수 = %lld, 결과 = %s\n",
                cmp_sorted, (ok2 ? "성공" : "실패(중복 또는 공간 부족)"));
            printf("AVL 트리 삽입:    비교 횟수 = %lld (중복 ID인 경우 실제로는 삽입되지 않을 수 있음)\n\n",
                cmp_avl);
        }
        else if (menu == 3) {
            // ----------- 삭제(Delete) -----------
            int key;
            printf("삭제할 학생 ID를 입력하세요: ");
            scanf("%d", &key);

            long long cmp_unsorted = 0, cmp_sorted = 0, cmp_avl = 0;

            int ok1 = delete_unsorted(unsorted, &n_unsorted, key, &cmp_unsorted);
            int ok2 = delete_sorted(sorted, &n_sorted, key, &cmp_sorted);
            avl_root = avl_delete(avl_root, key, &cmp_avl);

            printf("\n[삭제 결과] (ID=%d)\n", key);
            printf("비정렬 배열 삭제: 비교 횟수 = %lld, 결과 = %s\n",
                cmp_unsorted, (ok1 ? "성공" : "실패(해당 ID 없음)"));
            printf("정렬 배열 삭제:   비교 횟수 = %lld, 결과 = %s\n",
                cmp_sorted, (ok2 ? "성공" : "실패(해당 ID 없음)"));
            printf("AVL 트리 삭제:    비교 횟수 = %lld (해당 노드가 없으면 구조 변화 없음)\n\n",
                cmp_avl);
        }
        else {
            printf("잘못된 입력입니다. 다시 선택하세요.\n\n");
        }
    }

    // 메모리 해제
    free(original);
    free(unsorted);
    free(sorted);
    avl_free(avl_root);

    return 0;
}
