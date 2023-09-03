#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "json/json.h"

extern int dbg;

void json_start(char *filename, fcd_t *fcd) {
    jelement_t *root;
    short reclen;
    char record[MAX_REC_LEN+1];
    char *p, *key, *start;

    reclen = getshort(fcd->rec_len);
    memcpy(record, fcd->record, reclen);
    record[reclen] = 0;
    if ((p = strchr(record, ' ')) != NULL) *p = 0;

    if (dbg > 0) {
        fprintf(stderr, "********************* JSON start [%s] [%s]\n", filename, record);
    }
    root = (jelement_t *) getint(fcd->json_root);

    start = record;
    key = record;
    for (;;) {
        p = strchr(start, ':');
        if (p != NULL) {
            start = p + 1;
            *p = 0;
        }
        memcpy(fcd->status, ST_OK, 2);
        if (key[0]) {
            fprintf(stderr, "\tkey = [%s]\n", key);
            memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
            for (list2_t *ptr = list2_first(root->children); ptr != NULL; ptr = ptr->next) {
                jelement_t *child = (jelement_t *) ptr->buf;
                if (child->name != NULL && !strcmp(child->name, key)) {
                    root = child;
                    fprintf(stderr, "\tfound = %c [%s]\n", root->tp, root->name);
                    memcpy(fcd->status, ST_OK, 2);
                    break;
                }
            }
            if (memcmp(fcd->status, ST_OK, 2)) {
                return;
            }
        }
        if (p == NULL) {
            break;
        }
        key = start; 
    }
    putint(fcd->json_ptr, (int) root);
    putshort(fcd->json_ptr+4, 0);
}
