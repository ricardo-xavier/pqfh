#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "json/json.h"

extern int dbg;

void json_close(char *filename, fcd_t *fcd) {
    jelement_t *root;

    if (dbg > 0) {
        fprintf(stderr, "********************* JSON close [%s]\n", filename);
    }
    root = (jelement_t *) getint(fcd->json_root);
    free(root);
}
