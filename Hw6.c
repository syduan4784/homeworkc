// file: graph_compare.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define N 100
#define SPARSE_E 100
#define DENSE_E 4000

typedef struct Node {
    int v;
    struct Node* next;
} Node;

typedef struct {
    int n;
    uint8_t* adj; // n*n matrix (0/1)
} GraphMatrix;

typedef struct {
    int n;
    Node** head; // size n
    size_t node_count; // how many Node allocated (for memory calc)
} GraphList;

typedef struct {
    unsigned long long insert_cmp;
    unsigned long long delete_cmp;
    unsigned long long check_cmp;
    unsigned long long print_cmp;
} CmpCounter;

/* ---------- Helpers for RNG & edge generation ---------- */

typedef struct { int u, v; } Edge;

static void shuffle_edges(Edge* arr, size_t m) {
    for (size_t i = m - 1; i > 0; --i) {
        size_t j = (size_t)(rand() % (int)(i + 1));
        Edge tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

// Generate exactly E unique undirected edges among n vertices, no self-loops
static Edge* generate_random_edge_set(int n, int E) {
    size_t all = (size_t)n * (n - 1) / 2;
    Edge* pool = (Edge*)malloc(all * sizeof(Edge));
    size_t idx = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            pool[idx++] = (Edge){ i, j };
    shuffle_edges(pool, all);
    Edge* chosen = (Edge*)malloc(E * sizeof(Edge));
    for (int k = 0; k < E; ++k) chosen[k] = pool[k];
    free(pool);
    return chosen;
}

/* ---------- Graph (Matrix) ---------- */

static GraphMatrix* gm_create(int n) {
    GraphMatrix* g = (GraphMatrix*)malloc(sizeof(GraphMatrix));
    g->n = n;
    g->adj = (uint8_t*)calloc((size_t)n * n, sizeof(uint8_t));
    return g;
}

static void gm_free(GraphMatrix* g) {
    if (!g) return;
    free(g->adj);
    free(g);
}

static void gm_add_edge(GraphMatrix* g, int u, int v, CmpCounter* cc) {
    // count one comparison to check existing flag
    cc->insert_cmp++;
    if (g->adj[(size_t)u * g->n + v] == 0) {
        g->adj[(size_t)u * g->n + v] = 1;
        g->adj[(size_t)v * g->n + u] = 1;
    }
}

static void gm_remove_edge(GraphMatrix* g, int u, int v, CmpCounter* cc) {
    cc->delete_cmp++;
    if (g->adj[(size_t)u * g->n + v] == 1) {
        g->adj[(size_t)u * g->n + v] = 0;
        g->adj[(size_t)v * g->n + u] = 0;
    }
}

static int gm_has_edge(GraphMatrix* g, int u, int v, CmpCounter* cc) {
    cc->check_cmp++;
    return g->adj[(size_t)u * g->n + v] != 0;
}

static void gm_print_neighbors(GraphMatrix* g, int u, CmpCounter* cc) {
    // comparisons = n checks of adj[u][j]
    for (int j = 0; j < g->n; ++j) {
        cc->print_cmp++;
        if (g->adj[(size_t)u * g->n + j]) {
            // printf("%d ", j); // suppressed to avoid huge output
        }
    }
    // printf("\n");
}

static size_t gm_memory_bytes(GraphMatrix* g) {
    return sizeof(GraphMatrix) + (size_t)g->n * g->n * sizeof(uint8_t);
}

/* ---------- Graph (Adjacency List) ---------- */

static GraphList* gl_create(int n) {
    GraphList* g = (GraphList*)malloc(sizeof(GraphList));
    g->n = n;
    g->head = (Node**)calloc(n, sizeof(Node*));
    g->node_count = 0;
    return g;
}

static void gl_free(GraphList* g) {
    if (!g) return;
    for (int i = 0; i < g->n; ++i) {
        Node* cur = g->head[i];
        while (cur) {
            Node* nx = cur->next;
            free(cur);
            cur = nx;
        }
    }
    free(g->head);
    free(g);
}

static int gl_has_edge_oneway(GraphList* g, int u, int v, unsigned long long* cmp_slot) {
    Node* cur = g->head[u];
    while (cur) {
        (*cmp_slot)++;                // compare node->v with v
        if (cur->v == v) return 1;
        cur = cur->next;
    }
    return 0;
}

static void gl_add_edge(GraphList* g, int u, int v, CmpCounter* cc) {
    // ensure no duplicate (count comparisons while searching both lists)
    if (!gl_has_edge_oneway(g, u, v, &cc->insert_cmp)) {
        Node* a = (Node*)malloc(sizeof(Node));
        a->v = v; a->next = g->head[u]; g->head[u] = a; g->node_count++;
        Node* b = (Node*)malloc(sizeof(Node));
        b->v = u; b->next = g->head[v]; g->head[v] = b; g->node_count++;
    }
}

static void gl_remove_edge(GraphList* g, int u, int v, CmpCounter* cc) {
    // remove v from u's list
    Node** pc = &g->head[u];
    while (*pc) {
        cc->delete_cmp++;            // compare (*pc)->v with v
        if ((*pc)->v == v) {
            Node* del = *pc;
            *pc = del->next;
            free(del);
            g->node_count--;
            break;
        }
        pc = &(*pc)->next;
    }
    // remove u from v's list
    pc = &g->head[v];
    while (*pc) {
        cc->delete_cmp++;            // compare (*pc)->v with u
        if ((*pc)->v == u) {
            Node* del = *pc;
            *pc = del->next;
            free(del);
            g->node_count--;
            break;
        }
        pc = &(*pc)->next;
    }
}

static int gl_has_edge(GraphList* g, int u, int v, CmpCounter* cc) {
    return gl_has_edge_oneway(g, u, v, &cc->check_cmp);
}

static void gl_print_neighbors(GraphList* g, int u, CmpCounter* cc) {
    Node* cur = g->head[u];
    while (cur) {
        cc->print_cmp++;  // one comparison counted per neighbor visited
        // printf("%d ", cur->v);
        cur = cur->next;
    }
    // printf("\n");
}

static size_t gl_memory_bytes(GraphList* g) {
    // heads + nodes
    return sizeof(GraphList) + (size_t)g->n * sizeof(Node*) + g->node_count * sizeof(Node);
}

/* ---------- Build graphs from edge set ---------- */

static GraphMatrix* build_matrix_from_edges(int n, Edge* edges, int E) {
    GraphMatrix* gm = gm_create(n);
    CmpCounter tmp = { 0 };
    for (int i = 0; i < E; ++i) gm_add_edge(gm, edges[i].u, edges[i].v, &tmp);
    return gm;
}

static GraphList* build_list_from_edges(int n, Edge* edges, int E) {
    GraphList* gl = gl_create(n);
    CmpCounter tmp = { 0 };
    for (int i = 0; i < E; ++i) gl_add_edge(gl, edges[i].u, edges[i].v, &tmp);
    return gl;
}

/* ---------- Experiment runners ---------- */

static void make_random_pair(int n, int* u, int* v) {
    do {
        *u = rand() % n;
        *v = rand() % n;
    } while (*u == *v);
    if (*u > *v) { int t = *u; *u = *v; *v = t; }
}

static void run_case_matrix(const char* title, int n, Edge* base_edges, int E) {
    GraphMatrix* g = build_matrix_from_edges(n, base_edges, E);
    CmpCounter cc = { 0 };

    // 1) Memory
    size_t mem = gm_memory_bytes(g);

    // 2) Insert/Delete comparisons: do T trials with non-existing random edges (insert then delete)
    const int Tio = 100;
    for (int t = 0; t < Tio; ++t) {
        int u, v;
        // find a non-edge to insert (limit attempts)
        for (int tries = 0; tries < 10000; ++tries) {
            make_random_pair(n, &u, &v);
            CmpCounter tmp = { 0 };
            if (!gm_has_edge(g, u, v, &tmp)) { // count check comps in cc.insert_cmp? keep separate: not required
                // add then remove (measure comparisons inside add/remove)
                gm_add_edge(g, u, v, &cc);
                gm_remove_edge(g, u, v, &cc);
                break;
            }
        }
    }

    // 3) Connectivity check comparisons: random pairs
    const int Tc = 100;
    for (int t = 0; t < Tc; ++t) {
        int u, v; make_random_pair(n, &u, &v);
        gm_has_edge(g, u, v, &cc);
    }

    // 4) Print neighbors comparisons: random nodes
    const int Tp = 10;
    for (int t = 0; t < Tp; ++t) {
        int u = rand() % n;
        gm_print_neighbors(g, u, &cc);
    }

    printf("%s\n", title);
    printf("Memory: %zu Bytes\n", mem);
    printf("Edge insert/delete comparisons (total over %d ops): %llu\n", Tio * 2, cc.insert_cmp + cc.delete_cmp);
    printf("Connectivity checks (total over %d ops): %llu\n", Tc, cc.check_cmp);
    printf("Print neighbors comparisons (total over %d ops): %llu\n", Tp, cc.print_cmp);
    printf("\n");

    gm_free(g);
}

static void run_case_list(const char* title, int n, Edge* base_edges, int E) {
    GraphList* g = build_list_from_edges(n, base_edges, E);
    CmpCounter cc = { 0 };

    // 1) Memory
    size_t mem = gl_memory_bytes(g);

    // 2) Insert/Delete comparisons: do T trials with non-existing edges
    const int Tio = 100;
    for (int t = 0; t < Tio; ++t) {
        int u, v;
        // choose a non-edge to insert, ensure uniqueness
        for (int tries = 0; tries < 10000; ++tries) {
            make_random_pair(g->n, &u, &v);
            CmpCounter tmp = { 0 };
            if (!gl_has_edge(g, u, v, &tmp)) {
                gl_add_edge(g, u, v, &cc);
                gl_remove_edge(g, u, v, &cc);
                break;
            }
        }
    }

    // 3) Connectivity check comparisons
    const int Tc = 100;
    for (int t = 0; t < Tc; ++t) {
        int u, v; make_random_pair(g->n, &u, &v);
        gl_has_edge(g, u, v, &cc);
    }

    // 4) Print neighbors comparisons
    const int Tp = 10;
    for (int t = 0; t < Tp; ++t) {
        int u = rand() % g->n;
        gl_print_neighbors(g, u, &cc);
    }

    printf("%s\n", title);
    printf("Memory: %zu Bytes\n", mem);
    printf("Edge insert/delete comparisons (total over %d ops): %llu\n", Tio * 2, cc.insert_cmp + cc.delete_cmp);
    printf("Connectivity checks (total over %d ops): %llu\n", Tc, cc.check_cmp);
    printf("Print neighbors comparisons (total over %d ops): %llu\n", Tp, cc.print_cmp);
    printf("\n");

    gl_free(g);
}

int main(void) {
    srand((unsigned)time(NULL));

    // SPARSE (100 edges)
    Edge* sparse = generate_random_edge_set(N, SPARSE_E);
    // DENSE (4000 edges)
    Edge* dense = generate_random_edge_set(N, DENSE_E);

    // Build & run 4 cases using the SAME edge sets for fairness
    run_case_matrix("Case 1: Sparse + Adjacency Matrix", N, sparse, SPARSE_E);
    run_case_list("Case 2: Sparse + Adjacency List", N, sparse, SPARSE_E);
    run_case_matrix("Case 3: Dense  + Adjacency Matrix", N, dense, DENSE_E);
    run_case_list("Case 4: Dense  + Adjacency List", N, dense, DENSE_E);

    free(sparse);
    free(dense);
    return 0;
}
