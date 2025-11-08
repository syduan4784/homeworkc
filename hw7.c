#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10        // number of vertices
#define M 20        // number of edges (undirected)
#define INF 1e9

// Adjacency matrix, adj[u][v] = 1 if edge (u, v) exists
int adj[N][N];

// Simple queue structure for BFS
typedef struct {
    int data[N];
    int front, back;
} Queue;

void q_init(Queue* q) { q->front = q->back = 0; }
int q_empty(Queue* q) { return q->front == q->back; }
void q_push(Queue* q, int x) { q->data[q->back++] = x; }
int q_pop(Queue* q) { return q->data[q->front++]; }

// Generate a random undirected graph with N vertices and M edges
// No self-loops and no duplicate edges
void make_random_graph(unsigned int seed) {
    // reset adjacency matrix
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            adj[i][j] = 0;

    if (seed == 0) seed = (unsigned int)time(NULL);
    srand(seed);

    int edges = 0;
    while (edges < M) {
        int u = rand() % N;
        int v = rand() % N;
        if (u == v) continue;          // skip self-loops
        if (adj[u][v]) continue;       // skip duplicate edges
        adj[u][v] = adj[v][u] = 1;
        edges++;
    }
}

// BFS from node s to get distances and predecessors (for path reconstruction)
void bfs(int s, int dist[N], int prev[N]) {
    for (int i = 0; i < N; ++i) {
        dist[i] = (int)INF;
        prev[i] = -1;
    }
    Queue q; q_init(&q);
    dist[s] = 0;
    q_push(&q, s);

    while (!q_empty(&q)) {
        int u = q_pop(&q);
        for (int v = 0; v < N; ++v) {
            if (adj[u][v] && dist[v] == (int)INF) {
                dist[v] = dist[u] + 1;
                prev[v] = u;
                q_push(&q, v);
            }
        }
    }
}

// Print a path from s -> t based on the predecessor array
void print_path(int s, int t, int prev[N]) {
    // build the path in reverse order
    int path[N], len = 0;
    int cur = t;
    while (cur != -1) {
        path[len++] = cur;
        if (cur == s) break;
        cur = prev[cur];
    }
    if (cur == -1) {
        printf("  Path: (does not exist)\n");
        return;
    }
    // print in forward order
    printf("  Path: ");
    for (int i = len - 1; i >= 0; --i) {
        printf("%d", path[i]);
        if (i) printf(" -> ");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    // You can pass a seed as an argument: e.g. ./a.out 123
    unsigned int seed = 0;
    if (argc >= 2) {
        seed = (unsigned int)strtoul(argv[1], NULL, 10);
    }

    make_random_graph(seed);

    // Print adjacency matrix
    printf("Adjacency matrix (%dx%d):\n", N, N);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%d ", adj[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    // For each pair (i, j), i < j: run BFS from i and print shortest path to j
    int dist[N], prev[N];
    int pair_count = 0;
    for (int i = 0; i < N; ++i) {
        bfs(i, dist, prev);
        for (int j = i + 1; j < N; ++j) {
            pair_count++;
            printf("(%2d/%2d) Shortest path %d -> %d:\n",
                pair_count, (N * (N - 1)) / 2, i, j);
            if (dist[j] == (int)INF) {
                printf("  Distance: INF (no path exists)\n");
                print_path(i, j, prev);
            }
            else {
                printf("  Distance: %d (number of edges)\n", dist[j]);
                print_path(i, j, prev);
            }
            printf("\n");
        }
    }

    return 0;
}
