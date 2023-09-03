#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "json/json.h"

extern int dbg;

void json_next(char *filename, fcd_t *fcd) {
    jelement_t *parent;
    short idx, i;

    parent = (jelement_t *) getint(fcd->json_ptr);
    idx = getshort(fcd->json_idx);
    if (dbg > 0) {
        if (parent != NULL) {
            if (parent->name != NULL) {
                fprintf(stderr, "********************* JSON next [%s] %c [%s] %d\n", 
                    filename, parent->tp, parent->name, idx);
            } else {
                fprintf(stderr, "********************* JSON next [%s] %c NULL %d\n", 
                    filename, parent->tp, idx);
            }
        }
    }
    for (list2_t *ptr = list2_first(parent->children), i = 0; ptr != NULL; ptr = ptr->next, i++) {
        if (i > idx) {
            memcpy(fcd->status, ST_EOF, 2);
            break;
        }
        if (i < idx) {
            continue;
        }
        jelement_t *child = (jelement_t *) ptr->buf;
        if (dbg > 0) {
            fprintf(stderr, "\t%c %s=%s\n", child->tp, child->name, child->value);
        }
        putshort(fcd->json_idx, ++idx);
    }
}
