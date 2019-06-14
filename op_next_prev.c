#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

void op_next_prev(PGconn *conn, fcd_t *fcd, char dir) {

    unsigned int fileid;
    unsigned short reclen, keylen;
    table_t      *tab;
    char         sql[257], kbuf[MAX_KEY_LEN+1];
    PGresult     *res;

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;

    if (dbg > 0) {
        if (dir == 'n') {
            fprintf(stderr, "op_read_next [%s]\n", tab->name);
        } else {
            fprintf(stderr, "op_read_prev [%s]\n", tab->name);
        }
    }

    if (tab->restart == dir) {
        if (dir == 'n') {
            memcpy(fcd->record, tab->rec_next, reclen);
        } else {
            memcpy(fcd->record, tab->rec_prev, reclen);
        }
        memcpy(fcd->status, ST_OK, 2);
        if (dbg > 0) {
            if (dbg > 2) {
                fprintf(stderr, "[%s]\n", fcd->record);
            }
            fprintf(stderr, "restart status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        tab->restart = 0;
        return;
    }
    tab->restart = 0;

    sprintf(sql, "fetch next in cursor_%s_%ld", tab->name, tab->timestamp);
    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }
    res = PQexec(conn, sql);
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
        memcpy(fcd->status, ST_EOF, 2);
        if (dbg > 0) {
            fprintf(stderr, "%s\n", PQerrorMessage(conn));
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return;
    }

    pq2cob(tab, res, fcd->record, reclen);
    memcpy(fcd->status, ST_OK, 2);

    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
/*
    fprintf(stderr, "    [%s] [%s]\n",
        PQgetvalue(res, 0, 0),
        PQgetvalue(res, 0, 1));
*/

    unsigned short keyid = getshort(fcd->key_id);
    strcpy(kbuf, getkbuf(fcd, keyid, tab, &keylen));
    if (dir == 'n') {
        memcpy(tab->buf_next, kbuf, keylen);
        tab->buf_next[keylen] = 0;
        memcpy(tab->rec_next, fcd->record, reclen);
        tab->rec_next[reclen] = 0;
    } else {
        memcpy(tab->buf_prev, kbuf, keylen);
        tab->buf_prev[keylen] = 0;
        memcpy(tab->rec_prev, fcd->record, reclen);
        tab->rec_prev[reclen] = 0;
    }
}
