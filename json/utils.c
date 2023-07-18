#include "json.h"

char *tab(int n) {
    static char tab[17];
    for (int i = 0; i < n; i++) {
        tab[i] = '\t';
    }
    tab[n] = 0;
    return tab;
}

bool isWhiteSpace(char ch) {
    return ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ';
}
