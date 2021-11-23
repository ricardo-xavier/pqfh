#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "pqfh.h"

extern int dbg;
extern int dbg_times;
extern bool eof_start;

void op_next_prev(PGconn *conn, fcd_t *fcd, char dir) {
    funcao = _OP_NEXT_PREV;    

    unsigned int fileid;
    unsigned short reclen, keylen;
    table_t      *tab;
    char         sql[257], kbuf[MAX_KEY_LEN+1];
    PGresult     *res;
    struct timeval tv1, tv2, tv3;

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_READ, 2);
        if (dbg > 0) {
            short fnlen = getshort(fcd->file_name_len);
            char filename[257];
            memcpy(filename, (char *) fcd->file_name, fnlen);
            filename[fnlen] = 0;
            if (log_table(filename)) {
                fprintf(flog, "%ld op_next_prev [%s] %d\n", time(NULL), filename, (int) fcd->open_mode);
                fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
            }    
        }
        return;
    }

    if (dbg_times > 1) {
        gettimeofday(&tv1, NULL);
    }

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;

    if (dbg > 0) {
        if (dir == 'n') {
            if (log_table(tab->name)) fprintf(flog, "%ld op_read_next [%s]\n", time(NULL), tab->name);
        } else {
            if (log_table(tab->name)) fprintf(flog, "%ld op_read_prev [%s]\n", time(NULL), tab->name);
        }
    }

    if (tab->first) {
        tab->first = false;
        memcpy(fcd->status, ST_OK, 2);
        if (dbg > 0) {
            if (log_table(tab->name)) fprintf(flog, "%ld first status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return;
    }

    if (eof_start) {
        memcpy(fcd->status, ST_EOF, 2);
        if (dbg > 0) {
            if (log_table(tab->name)) fprintf(flog, "%ld eof status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return;
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
                if (log_table(tab->name)) fprintf(flog, "%ld [%s]\n", time(NULL), fcd->record);
            }
            if (log_table(tab->name)) fprintf(flog, "%ld restart status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        tab->restart = 0;
        return;
    }
    tab->restart = 0;

    sprintf(sql, "fetch next in cursor_%s_%ld", tab->name, tab->timestamp);
    if (dbg > 1) {
        if (log_table(tab->name)) fprintf(flog, "%ld %s\n", time(NULL), sql);
    }
    res = PQexec(conn, sql);
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
        memcpy(fcd->status, ST_EOF, 2);
        if (dbg > 0) {
            if (log_table(tab->name)) fprintf(flog, "%ld %s\n", time(NULL), PQerrorMessage(conn));
            if (log_table(tab->name)) fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        PQclear(res);
        return;
    }

    if (dbg_times > 1) {
        gettimeofday(&tv2, NULL);
    }

    pq2cob(tab, res, fcd->record, reclen);
    PQclear(res);
    memcpy(fcd->status, ST_OK, 2);

    if (dbg_times > 1) {
        gettimeofday(&tv3, NULL);
        long tempo1 = ((tv3.tv_sec * 1000000) + tv3.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        long tempo2 = ((tv3.tv_sec * 1000000) + tv3.tv_usec) - ((tv2.tv_sec * 1000000) + tv2.tv_usec);
        if (log_table(tab->name)) fprintf(flog, "%ld op_next %c [%s] tempo=%ld %ld\n", time(NULL), dir, tab->name, tempo1, tempo2);
    }

    if (dbg > 0) {
        if (log_table(tab->name)) fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
    }
/*
    fprintf(flog, "    [%s] [%s]\n",
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
