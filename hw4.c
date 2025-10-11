#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 100
#define MAXV 1000

typedef struct Node {
    int key;
    struct Node* left, * right;
} Node;

/* ----- Comparison counter ----- */
typedef struct {
    long long comparisons;
} Counter;

/* ----- Create a new BST node ----- */
Node* new_node(int key) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->key = key;
    n->left = n->right = NULL;
    return n;
}

/* ----- Insert into BST (<= goes to the right for duplicates) ----- */
Node* bst_insert(Node* root, int key) {
    if (!root) return new_node(key);
    if (key < root->key) root->left = bst_insert(root->left, key);
    else                  root->right = bst_insert(root->right, key);
    return root;
}

/* ----- Search in BST with comparison count ----- */
int bst_search(Node* root, int key, Counter* c) {
    while (root) {
        c->comparisons++;                 // compare with root->key
        if (key == root->key) return 1;
        else if (key < root->key) root = root->left;
        else root = root->right;
    }
    return 0;
}

/* ----- Free all nodes in BST ----- */
void bst_free(Node* root) {
    if (!root) return;
    bst_free(root->left);
    bst_free(root->right);
    free(root);
}

/* ----- Linear search in array with comparison count ----- */
int linear_search(int* a, int n, int key, Counter* c) {
    for (int i = 0; i < n; i++) {
        c->comparisons++;                 // compare a[i] with key
        if (a[i] == key) return 1;
    }
    return 0;
}

/* ----- Utility: calculate elapsed time in seconds ----- */
double elapsed_seconds(clock_t start, clock_t end) {
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main(void) {
    srand((unsigned)time(NULL));

    int arr[N];
    Node* root = NULL;

    // 1) Generate random numbers & 2) Build the BST
    for (int i = 0; i < N; i++) {
        arr[i] = rand() % (MAXV + 1); // [0..1000]
        root = bst_insert(root, arr[i]);
    }

    // 3) Pick a target that exists in the array
    int idx = rand() % N;
    int target_hit = arr[idx];

    // (optional) Pick a target that does not exist
    int target_miss = MAXV + 1; // 1001

    // 4) Linear search & BST search (case: target found)
    Counter linC = { 0 }, bstC = { 0 };
    clock_t t1, t2;

    t1 = clock();
    int linFound = linear_search(arr, N, target_hit, &linC);
    t2 = clock();
    double linTime = elapsed_seconds(t1, t2);

    t1 = clock();
    int bstFound = bst_search(root, target_hit, &bstC);
    t2 = clock();
    double bstTime = elapsed_seconds(t1, t2);

    printf("=== Case: target EXISTS (target = %d) ===\n", target_hit);
    printf("Linear Search: found=%d, comparisons=%lld, time=%.9f s\n",
        linFound, linC.comparisons, linTime);
    printf("BST Search   : found=%d, comparisons=%lld, time=%.9f s\n",
        bstFound, bstC.comparisons, bstTime);

    // 5) (Optional) Compare case: target not found
    Counter linC2 = { 0 }, bstC2 = { 0 };

    t1 = clock();
    int linFound2 = linear_search(arr, N, target_miss, &linC2);
    t2 = clock();
    double linTime2 = elapsed_seconds(t1, t2);

    t1 = clock();
    int bstFound2 = bst_search(root, target_miss, &bstC2);
    t2 = clock();
    double bstTime2 = elapsed_seconds(t1, t2);

    printf("\n=== Case: target NOT EXISTS (target = %d) ===\n", target_miss);
    printf("Linear Search: found=%d, comparisons=%lld, time=%.9f s\n",
        linFound2, linC2.comparisons, linTime2);
    printf("BST Search   : found=%d, comparisons=%lld, time=%.9f s\n",
        bstFound2, bstC2.comparisons, bstTime2);

    // Free memory
    bst_free(root);
    return 0;
}
