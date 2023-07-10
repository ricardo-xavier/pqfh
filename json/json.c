#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

void load(char *filename, char **buf);
void parse(jobject_t obj, int i);

int main(int argc, char *argv[]) {
    char *json;
    jobject_t root;

    if (argc != 2) {
        fprintf(stderr, "Syntax: %s <file.json>\n", argv[0]);
        return -1;
    }
    load(argv[1], &json);
    printf("%s\n", json);
    strcpy(root.name, "/");
    parse(root, 0);
    free(json);
    return 0;
}

void parse(jobject_t obj, int i) {
}
