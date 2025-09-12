// hw1-2.c
//gcc -std=c11 -O2 -Wall -Wextra -o hw1-2.exe .\hw1-2.c
//.\hw1-2.exe
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAXN 10000

static const char *s;
static int i, n;
static int ok = 1;

static void skip_ws(void) {
    while (i < n && isspace((unsigned char)s[i])) i++;
}

static void parse_node(void) {
    skip_ws();
    if (i >= n) { ok = 0; return; }

  
    if (isalpha((unsigned char)s[i])) {
        i++; // consume label
        return;
    }

   
    if (s[i] != '(') { ok = 0; return; }
    i++; // '('
    skip_ws();

   
    if (i >= n || !isalpha((unsigned char)s[i])) { ok = 0; return; }
    i++; // consume label

    int child_cnt = 0;
    for (;;) {
        skip_ws();
        if (i >= n) { ok = 0; return; }

        if (s[i] == ')') {
            i++; // ')'
            break;
        }

        if (s[i] == '(' || isalpha((unsigned char)s[i])) {
            parse_node();
            child_cnt++;
            if (child_cnt > 2) { ok = 0; return; }
        } else {
            ok = 0; return;
        }
    }
}

int main(void) {
    static char buf[MAXN + 5];
    if (!fgets(buf, sizeof(buf), stdin)) return 0;

    s = buf;
    n = (int)strlen(buf);
    i = 0;
    ok = 1;

    skip_ws();
    parse_node(); 
    skip_ws();

    
    while (i < n && isspace((unsigned char)s[i])) i++;
    if (i < n) ok = 0;

    puts(ok ? "TRUE" : "FALSE");
    return 0;
}
