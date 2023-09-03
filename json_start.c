#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "json/json.h"

extern int dbg;

void json_start(char *filename, fcd_t *fcd) {
    //jelement_t *root;
    short reclen;
    char record[MAX_REC_LEN+1];
    char *p;

    reclen = getshort(fcd->rec_len);
    memcpy(record, fcd->record, reclen);
    record[reclen] = 0;
    if ((p = strchr(record, ' ')) != NULL) *p = 0;

    fprintf(stderr, "********************* JSON start [%s] [%s]\n", filename, record);
    //root = (jelement_t *) getint(fcd->json_root);
}
