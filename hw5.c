#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000
#define MAXV 10000

/*============================
 *  AVL / BST node
 *============================*/
typedef struct Node {
    int key;
    int height;           /* used by AVL; ignored for plain BST */
    struct Node* left;
    struct Node* right;
} Node;

/* ---- helpers: không dùng inline để hợp MSVC C89 ---- */
static int mymax(int a, int b) { return a > b ? a : b; }
static int height(Node* n) { return n ? n->height : 0; }

Node* new_node(int key) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) { fprintf(stderr, "malloc failed\n"); exit(1); }
    n->key = key; n->height = 1; n->left = NULL; n->right = NULL;
    return n;
}

/*============================
 *  BST insert (no rebalancing)
 *============================*/
Node* bst_insert(Node* root, int key) {
    if (!root) return new_node(key);
    if (key < root->key) root->left = bst_insert(root->left, key);
    else if (key > root->key) root->right = bst_insert(root->right, key);
    return root; /* keys unique; ignore duplicates */
}

/*============================
 *  AVL rotations & insert
 *============================*/
Node* rotate_right(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;
    x->right = y; y->left = T2;
    y->height = 1 + mymax(height(y->left), height(y->right));
    x->height = 1 + mymax(height(x->left), height(x->right));
    return x;
}

Node* rotate_left(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;
    y->left = x; x->right = T2;
    x->height = 1 + mymax(height(x->left), height(x->right));
    y->height = 1 + mymax(height(y->left), height(y->right));
    return y;
}

Node* avl_insert(Node* node, int key) {
    int balance;
    if (!node) return new_node(key);

    if (key < node->key) node->left = avl_insert(node->left, key);
    else if (key > node->key) node->right = avl_insert(node->right, key);
    else return node; /* dup */

    node->height = 1 + mymax(height(node->left), height(node->right));
    balance = height(node->left) - height(node->right);

    /* LL */
    if (balance > 1 && key < node->left->key)
        return rotate_right(node);
    /* RR */
    if (balance < -1 && key > node->right->key)
        return rotate_left(node);
    /* LR */
    if (balance > 1 && key > node->left->key) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    /* RL */
    if (balance < -1 && key < node->right->key) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

/*============================
 *  Search counting (== comparisons)
 *============================*/
int array_linear_search_count(const int* arr, int x) {
    int i, cnt = 0;
    for (i = 0; i < N; i++) {
        cnt++;                    /* compare arr[i] == x */
        if (arr[i] == x) return cnt;
    }
    return cnt; /* not found after N equality checks */
}

int bst_search_count(Node* root, int x) {
    int cnt = 0;
    while (root) {
        cnt++;                    /* compare root->key == x */
        if (x == root->key) return cnt;
        if (x < root->key) root = root->left;
        else root = root->right;
    }
    return cnt; /* not found; number of visited nodes */
}

/*============================
 *  Utilities
 *============================*/
void free_tree(Node* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

/* dùng unsigned char thay bool để hợp chuẩn C cũ */
void make_dataset1_unique_random(int out[N]) {
    unsigned char used[MAXV + 1] = { 0 };
    int filled = 0;
    while (filled < N) {
        int v = rand() % (MAXV + 1); /* 0..10000 */
        if (!used[v]) { used[v] = 1; out[filled++] = v; }
    }
}

void make_dataset2_sorted_inc(int out[N]) {
    int i;
    for (i = 0; i < N; i++) out[i] = i; /* 0..999 */
}

void make_dataset3_sorted_dec(int out[N]) {
    int i;
    for (i = 0; i < N; i++) out[i] = N - 1 - i; /* 999..0 */
}

void make_dataset4_formula(int out[N]) {
    int i;
    for (i = 0; i < N; i++) out[i] = i * ((i % 2) + 2); /* even: 2i, odd: 3i */
}

void build_structures_from_data(const int data_in[N], int array_out[N],
    Node** bst_root, Node** avl_root) {
    int i;
    for (i = 0; i < N; i++) array_out[i] = data_in[i];
    *bst_root = NULL; *avl_root = NULL;
    for (i = 0; i < N; i++) {
        *bst_root = bst_insert(*bst_root, data_in[i]);
        *avl_root = avl_insert(*avl_root, data_in[i]);
    }
}

/* Run 1000 queries and print averages */
void run_queries_and_report(const int array_data[N], Node* bst_root, Node* avl_root,
    const char* dataset_name) {
    long long sum_array = 0, sum_bst = 0, sum_avl = 0;
    int q;
    for (q = 0; q < N; q++) {
        int x = rand() % (MAXV + 1); /* 0..10000 */
        sum_array += array_linear_search_count(array_data, x);
        sum_bst += bst_search_count(bst_root, x);
        sum_avl += bst_search_count(avl_root, x); /* same counting logic */
    }
    printf("Array: 데이터 %s에서 평균 %.2f회 탐색\n", dataset_name, (double)sum_array / N);
    printf("BST:   데이터 %s에서 평균 %.2f회 탐색\n", dataset_name, (double)sum_bst / N);
    printf("AVL:   데이터 %s에서 평균 %.2f회 탐색\n", dataset_name, (double)sum_avl / N);
}

int main(void) {
    int i;
    int arr[N];
    int data[4][N];
    Node* bst = NULL;
    Node* avl = NULL;

    /* đặt seed sau KHỞI TẠO biến để hợp C89 */
    srand(20251020); /* hoặc: srand((unsigned)time(NULL)); */

    /* (1) unique random */
    make_dataset1_unique_random(data[0]);
    build_structures_from_data(data[0], arr, &bst, &avl);
    run_queries_and_report(arr, bst, avl, "(1)");
    free_tree(bst); free_tree(avl); bst = avl = NULL;

    /* (2) sorted increasing 0..999 */
    make_dataset2_sorted_inc(data[1]);
    build_structures_from_data(data[1], arr, &bst, &avl);
    run_queries_and_report(arr, bst, avl, "(2)");
    free_tree(bst); free_tree(avl); bst = avl = NULL;

    /* (3) sorted decreasing 999..0 */
    make_dataset3_sorted_dec(data[2]);
    build_structures_from_data(data[2], arr, &bst, &avl);
    run_queries_and_report(arr, bst, avl, "(3)");
    free_tree(bst); free_tree(avl); bst = avl = NULL;

    /* (4) value[i] = i * (i % 2 + 2) */
    make_dataset4_formula(data[3]);
    build_structures_from_data(data[3], arr, &bst, &avl);
    run_queries_and_report(arr, bst, avl, "(4)");
    free_tree(bst); free_tree(avl); bst = avl = NULL;

    return 0;
}
