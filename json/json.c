#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "json.h"

int main(int argc, char *argv[]) {
    char *json;
    int dbg = 0;
    jelement_t root;
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Syntax: %s <file.json> [-d[d]]\n", argv[0]);
        return -1;
    }
    if (argc == 3) {
        if (strcmp(argv[2], "-d") == 0) {
            dbg = 1;
        } else if (strcmp(argv[2], "-dd") == 0) {
            dbg = 2;
        }
    }
    load(argv[1], &json);
    memset(&root, 0, sizeof(jelement_t));
    parse(json, dbg, &root);
    free(json);
    json = tostring(dbg, root);
    printf("%s\n", json);
    free(json);
    return 0;
}
