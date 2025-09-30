#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXN 256  

int L[MAXN], R[MAXN];
char label_of[MAXN];
int id_of[256];
int lastChild[MAXN];
int nNodes = 0;
int root = 0;


int get_id(char c) {
    if (id_of[(unsigned char)c]) return id_of[(unsigned char)c];
    ++nNodes;
    id_of[(unsigned char)c] = nNodes;
    label_of[nNodes] = c;
    L[nNodes] = R[nNodes] = lastChild[nNodes] = 0;
    return nNodes;
}

void attach_child(int parent, int child) {
    if (parent == 0) return;
    if (L[parent] == 0) {
        L[parent] = child;
    }
    else {
        R[lastChild[parent]] = child;
    }
    lastChild[parent] = child;
}

/* ---------------- Iterative traversals ---------------- */
void preorder_iter(int r) {
    if (!r) return;
    int st[MAXN], top = 0;
    st[top++] = r;
    while (top) {
        int u = st[--top];
        printf("%c ", label_of[u]);
        if (R[u]) st[top++] = R[u];
        if (L[u]) st[top++] = L[u];
    }
}

void inorder_iter(int r) {
    int st[MAXN], top = 0, cur = r;
    while (cur || top) {
        while (cur) { st[top++] = cur; cur = L[cur]; }
        cur = st[--top];
        printf("%c ", label_of[cur]);
        cur = R[cur];
    }
}

void postorder_iter(int r) {
    if (!r) return;
    int s1[MAXN], s2[MAXN], t1 = 0, t2 = 0;
    s1[t1++] = r;
    while (t1) {
        int u = s1[--t1];
        s2[t2++] = u;
        if (L[u]) s1[t1++] = L[u];
        if (R[u]) s1[t1++] = R[u];
    }
    while (t2) {
        int u = s2[--t2];
        printf("%c ", label_of[u]);
    }
}

int main(void) {
    char line[4096];
    if (!fgets(line, sizeof(line), stdin)) return 0;


    int st[MAXN], top = 0;

    int expect_label_after_open = 0;
    for (int i = 0; line[i]; ++i) {
        char ch = line[i];
        if (isspace((unsigned char)ch)) continue;

        if (ch == '(') {
            expect_label_after_open = 1;
        }
        else if (ch == ')') {
            if (top) --top;
        }
        else if (isupper((unsigned char)ch)) {
            if (expect_label_after_open) {

                int v = get_id(ch);
                if (root == 0) {
                    root = v;
                }
                if (top > 0) attach_child(st[top - 1], v);
                st[top++] = v;
                expect_label_after_open = 0;
            }
            else {

                int v = get_id(ch);
                if (top > 0) attach_child(st[top - 1], v);
                else {

                    if (root == 0) root = v;
                }
            }
        }

    }


    printf("pre-order: ");  preorder_iter(root);  printf("\n");
    printf("in-order: ");   inorder_iter(root);   printf("\n");
    printf("post-order: "); postorder_iter(root); printf("\n");
    return 0;
}
