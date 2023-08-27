#include "pqfh.h"
#include "json/json.h"
#include <stdlib.h>
#include <string.h>

bool json_open(char *filename, fcd_t *fcd) {
    char *json = NULL;
    jelement_t root;
    int dbg = 0;    

    fprintf(stderr, "********************* TODO json_open [%s]\n", filename);
    if (!load(filename+5, &json)) {
        if (json != NULL) {
            free(json);
        }
        memcpy(fcd->status, ST_ERROR, 2);
        return false;
    } 

    memset(&root, 0, sizeof(jelement_t));
    if (!parse(json, dbg, &root)) {
        if (json != NULL) {
            free(json);
        }
        memcpy(fcd->status, ST_ERROR, 2);
        return false;
    }
    free(json);
    json = tostring(dbg, root);
    printf("%s\n", json);
    free(json);    
    memcpy(fcd->status, ST_OK, 2);
    return true;
}
