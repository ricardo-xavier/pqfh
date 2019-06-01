#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

void op_next_prev(PGconn *conn, fcd_t *fcd, char dir) {

    unsigned int fileid, keyoffset, keylen;
    unsigned short reclen;
    table_t      *tab;
    char         sql[257];
    PGresult     *res;

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    kdb(fcd, &keyoffset, &keylen);

    tab = (table_t *) fileid;

    if (dbg > 0) {
        if (dir == 'n') {
            fprintf(stderr, "op_read_next [%s]\n", tab->name);
        } else {
            fprintf(stderr, "op_read_prev [%s]\n", tab->name);
        }
    }

    sprintf(sql, "fetch next in cursor_%s", tab->name);
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

    if (dir == 'n') {
        memcpy(tab->buf_next, fcd->record+keyoffset, keylen);
        tab->buf_next[keylen] = 0;
    } else {
        memcpy(tab->buf_prev, fcd->record+keyoffset, keylen);
        tab->buf_prev[keylen] = 0;
    }
}
