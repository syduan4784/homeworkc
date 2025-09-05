#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAXN 100000

static const char *s;
static int i, n;
static int errorFlag = 0;
static int notBinary = 0;

void skip_ws() {
    while (i < n && isspace((unsigned char)s[i])) i++;
}

int parseNodeExists(); // forward

// Parse a leaf written as a single alpha label (returns 1 leaf consumed, 0 otherwise)
int parseLeafIfAny() {
    skip_ws();
    if (i < n && isalpha((unsigned char)s[i])) {
        // one-letter label
        i++;
        return 1; // leaf node exists
    }
    return 0; // not a leaf here
}

// Parse a child: could be () null, a '(' node ')', or a single-letter leaf
// returns: -1 error, 0 = null (no node), 1 = a real child node present
int parseChild() {
    skip_ws();
    if (i >= n) { errorFlag = 1; return -1; }

    if (s[i] == '(') {
        // Could be null "()" or a full node
        int start = i;
        i++; // consume '('
        skip_ws();
        if (i < n && s[i] == ')') {
            // null child
            i++; // consume ')'
            return 0; // null
        }
        // Not null: must be a node ⇒ backtrack to '(' and call parseNodeExists
        i = start;
        int ok = parseNodeExists();
        if (ok < 0) return -1;
        return 1; // real child
    } else if (isalpha((unsigned char)s[i])) {
        // leaf node (single label)
        i++;
        return 1;
    } else {
        // Unexpected token inside a node (before ')')
        // If it's a ')', caller will handle. Otherwise error.
        if (s[i] == ')') return -2; // signal "no more children" to caller
        errorFlag = 1;
        return -1;
    }
}

// Parse a node beginning with '(' ... ')' or detect null "()"
// Returns: -1 error, 0 = null node, 1 = real node
int parseNodeExists() {
    skip_ws();
    if (i >= n || s[i] != '(') { errorFlag = 1; return -1; }
    i++; // consume '('
    skip_ws();

    // null node: "()"
    if (i < n && s[i] == ')') {
        i++; // consume ')'
        return 0; // null
    }

    // must read a label (single alpha)
    if (i >= n || !isalpha((unsigned char)s[i])) { errorFlag = 1; return -1; }
    i++; // consume label
    skip_ws();

    int realChildren = 0;

    // read up to two children until we hit ')'
    while (1) {
        skip_ws();
        if (i >= n) { errorFlag = 1; return -1; }
        if (s[i] == ')') {
            i++; // end of this node
            break;
        }

        int res = parseChild();
        if (res == -1) return -1;           // error
        if (res == -2) { // saw ')', but we didn't consume it here; let loop handle
            continue;
        }
        if (res == 1) { // a real child
            realChildren++;
            if (realChildren > 2) {
                notBinary = 1;
                // keep consuming until ')', but we already count >2
            }
        }
        // res == 0 => null child: doesn't increase realChildren
        // Continue to check for next child or ')'
    }

    return 1; // real node parsed
}

int main(void) {
    static char buf[MAXN + 5];
    // Read entire line (the assignment says input is one line; treat empty/invalid as ERROR)
    if (!fgets(buf, sizeof(buf), stdin)) {
        printf("ERROR\n");
        return 0;
    }
    // Strip trailing newline(s) but keep internal spaces
    // Also detect if line is only whitespace
    int len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';

    // Check non-empty (non-whitespace)
    int onlyWS = 1;
    for (int k = 0; k < len; ++k) {
        if (!isspace((unsigned char)buf[k])) { onlyWS = 0; break; }
    }
    if (onlyWS) { printf("ERROR\n"); return 0; }

    s = buf; n = len; i = 0; errorFlag = 0; notBinary = 0;

    int res = parseNodeExists(); // parse the root
    skip_ws();

    if (errorFlag || res < 0) {
        printf("ERROR\n");
        return 0;
    }
    // Must consume entire string
    if (i != n) {
        // leftover garbage
        // allow trailing whitespace only:
        int k = i;
        while (k < n && isspace((unsigned char)s[k])) k++;
        if (k != n) { printf("ERROR\n"); return 0; }
    }

    if (res == 0) {
        // Entire input is "()" => technically a null tree; tùy quy ước. Ở đây coi là TRUE (không có nút nào vi phạm nhị phân)
        printf("TRUE\n");
    } else if (notBinary) {
        printf("FALSE\n");
    } else {
        printf("TRUE\n");
    }
    return 0;
}
