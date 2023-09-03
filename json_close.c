#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "json/json.h"

extern int dbg;

void json_close(char *filename, fcd_t *fcd) {
    char *json = NULL;
    jelement_t *root;

    fprintf(stderr, "********************* JSON json_close [%s]\n", filename);
    memcpy(root, fcd->root, 4);
    json = tostring(dbg, *root);
    printf("%s\n", json);
    free(json);    
}
