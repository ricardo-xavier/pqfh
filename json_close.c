#include "pqfh.h"
#include "json/json.h"
#include <stdlib.h>
#include <string.h>

bool json_close(fcd_t *fcd) {
    fprintf(stderr, "********************* JSON json_close [%s]\n", filename);
    if (!load(filename+5, &json)) {
        if (json != NULL) {
            free(json);
        }
        memcpy(fcd->status, ST_ERROR, 2);
        return false;
    } 

    root = malloc(sizeof(jelement_t))
    memset(root, 0, sizeof(jelement_t));
    if (!parse(json, dbg, root)) {
        if (json != NULL) {
            free(root);
            free(json);
        }
        memcpy(fcd->status, ST_ERROR, 2);
        return false;
    }

    memcpy(fcd->root, root, 4);
    free(json);
    json = tostring(dbg, root);
    printf("%s\n", json);
    free(json);    
    memcpy(fcd->status, ST_OK, 2);
    return true;
}
