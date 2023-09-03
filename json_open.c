#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "json/json.h"

extern int dbg;

void json_open(char *filename, fcd_t *fcd) {
    char *json = NULL;
    jelement_t *root;

    if (dbg > 0) {
        fprintf(stderr, "********************* JSON open [%s]\n", filename);
    }
    if (!load(filename, &json)) {
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

    putint(fcd->json_root, (int) root);
    putint(fcd->json_ptr, (int) root);
    putshort(fcd->json_idx, 0);
short idx = getshort(fcd->json_idx);
fprintf(stderr, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX %d\n", idx);
    free(json);
    memcpy(fcd->status, ST_OK, 2);
}
