#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "json/json.h"

extern int dbg;

void json_open(char *filename, fcd_t *fcd) {
    char *json = NULL;
    jelement_t *root;

    fprintf(stderr, "********************* JSON json_open [%s]\n", filename);
    if (!load(filename+5, &json)) {
        if (json != NULL) {
            free(json);
        }
        memcpy(fcd->status, ST_ERROR, 2);
        return;
    } 

    root = malloc(sizeof(jelement_t));
    memset(root, 0, sizeof(jelement_t));
    if (!parse(json, dbg, root)) {
        if (json != NULL) {
            free(root);
            free(json);
        }
        memcpy(fcd->status, ST_ERROR, 2);
        return;
    }

    memcpy(fcd->root, root, 4);
    free(json);
    memcpy(fcd->status, ST_OK, 2);
}
