#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "json/json.h"

extern int dbg;

void json_next(char *filename, fcd_t *fcd) {
    jelement_t *parent;
    short idx, i;

    parent = (jelement_t *) getint(fcd->json_ptr);
    idx = getshort(fcd->json_ptr+4);
    if (dbg > 0) {
        if (parent != NULL) {
            fprintf(stderr, "********************* JSON next [%s] %c [%s] %d\n", 
                filename, parent->tp, parent->name != NULL ? parent->name : "NULL", idx);
        }
    }
    i = 0;
    memcpy(fcd->status, ST_EOF, 2);
    for (list2_t *ptr = list2_first(parent->children); ptr != NULL; ptr = ptr->next) {
        if (i > idx) {
            break;
        }
        if (i < idx) {
            i++;
            continue;
        }
        jelement_t *child = (jelement_t *) ptr->buf;
        if (dbg > 0) {
            fprintf(stderr, "\t%c %s=%s\n", child->tp, 
                child->name != NULL ? child->name : "NULL",
                child->value != NULL ? child->value : "NULL");
        }
        putshort(fcd->json_ptr+4, ++idx);
        memcpy(fcd->status, ST_OK, 2);
        fcd->record[100] = child->tp;
        memset(fcd->record+101, ' ', 50);
        if (child->name != NULL) {
            memcpy(fcd->record+101, child->name, strlen(child->name)); 
        } else {
            memcpy(fcd->record+101, "NULL", 4);
        }
        memset(fcd->record+151, ' ', 250);
        if (child->value != NULL) {
            memcpy(fcd->record+151, child->value, strlen(child->value)); 
        } else {
            memcpy(fcd->record+151, "NULL", 4);
        }
        break;
    }
}
