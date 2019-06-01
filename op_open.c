#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;

void op_open(PGconn *conn, fcd_t *fcd, unsigned short opcode) {

    char           filename[257], aux[257], *p;
    table_t        *tab;
    unsigned short fnlen, reclen;
    unsigned int   fileid;

    fnlen = getshort(fcd->file_name_len);
    reclen = getshort(fcd->rec_len);
    memcpy(filename, (char *) fcd->file_name, fnlen);
    filename[fnlen] = 0;
    if ((p = strchr(filename, ' ')) != NULL) *p = 0;
    if ((p = strrchr(filename, '/')) != NULL) {
        strcpy(aux, p+1);
        strcpy(filename, aux);
    }
    
    if (dbg > 0) {
        fprintf(stderr, "op_open [%s] %04x %ld\n", filename, opcode, time(NULL));
    }

    tab = (table_t *) malloc(sizeof(table_t));
    strcpy(tab->name, filename);
    tab->read_prepared = false;
    tab->upd_prepared = false;
    tab->ins_prepared = false;
    tab->del_prepared = false;
    tab->key_next = -1;
    tab->key_prev = -1;
    tab->prms = NULL;
    if (table_info(conn, tab, fcd)) {
        memcpy(fcd->status, ST_OK, 2);
    } else {
        memcpy(fcd->status, ST_FILE_NOT_FOUND, 2);
    }

    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }

    fileid = (int) tab;
    putint(fcd->file_id, fileid);
}
