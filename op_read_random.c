#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "pqfh.h"

extern int dbg;
extern int dbg_times;
extern bool partial;

void op_read_random(PGconn *conn, fcd_t *fcd, bool with_lock) {

    unsigned int   fileid ;
    unsigned short keyid, reclen, keylen;
    char           kbuf[MAX_KEY_LEN+1], sql[257], stmt_name[65], where[4097];
    int            nParams, p;
    table_t        *tab;
    column_t       *col;
    PGresult       *res;
    list2_t        *ptr;
    bool           lock, lock_automatic, ignore_lock;
    struct timeval tv1, tv2, tv3;

    if (dbg_times > 1) {
        gettimeofday(&tv1, NULL);
    }

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "%ld op_read_random [%s] lock mode=%d ignore lock=%d\n", time(NULL), tab->name, fcd->lock_mode, fcd->ignore_lock);
    }

    if (tab->advisory_lock > 0) {
        unlock(fcd);
    }

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_READ, 2);
        if (dbg > 0) {
            fprintf(stderr, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return;
    }

/*
    bool lock_exclusive = (fcd->lock_mode & 0x01) == 0x01;
    bool lock_manual = (fcd->lock_mode & 0x01) == 0x01;
*/

    keyid = getshort(fcd->key_id);
    strcpy(kbuf, getkbuf(fcd, keyid, tab, &keylen));
    if (dbg > 1) {
        fprintf(stderr, "%ld key %d %d [%s]\n", time(NULL), keyid, keylen, kbuf);
    }
    sprintf(stmt_name, "%s_%ld_%d", tab->name, tab->timestamp, keyid);

    // prepara o comando se ainda nao tiver preparado
    if (!tab->read_prepared[keyid]) {

        ignore_lock = (fcd->ignore_lock & 0x01) == 0x01;
        lock_automatic = (fcd->lock_mode & 0x02) == 0x02;
        lock = (fcd->open_mode > 0) && lock_automatic && !ignore_lock;

        getwhere_prepared(tab, keyid, where, 0, 's');
        if (lock || with_lock) {
            if ((tab->oid > 0) && (atoi(kbuf) > 0)) {
                char result[9];
                sprintf(sql, "SELECT pg_try_advisory_lock(%d, %d)", tab->oid, atoi(kbuf));
                res = PQexec(conn, sql);
                if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
                    strcpy(result, "f");
                } else {
                    strcpy(result, PQgetvalue(res, 0, 0));
                }
                PQclear(res);
                if (dbg > 0) {
                    fprintf(stderr, "%ld advisory_lock(%d,%d)=[%s]\n", time(NULL), tab->oid, atoi(kbuf), result);
                }
                if (result[0] == 'f') {
                    memcpy(fcd->status, ST_LOCKED, 2); 
                    sleep(1);
                    if (dbg > 0) {
                        fprintf(stderr, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
                    }
                    return;
                }
                tab->advisory_lock = atoi(kbuf);
            }
        }
        sprintf(sql, "select * from %s.%s where %s", tab->schema, tab->name, where);
        nParams = list2_size(tab->prms_random[keyid]);

        if (dbg > 1) {
            fprintf(stderr, "%ld %s\n", time(NULL), sql);
        }

        res = PQprepare(conn, stmt_name, sql, nParams, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
        tab->read_prepared[keyid] = true;
    }

    // seta parametros
    p = 0;
    for (ptr=tab->prms_random[keyid]; ptr!=NULL; ptr=ptr->next) {
        col = (column_t *) ptr->buf;
        memcpy(tab->bufs[p], fcd->record+col->offset, col->len);
        tab->bufs[p][col->len] = 0;
        tab->values[p] = tab->bufs[p];
        tab->lengths[p] = col->len;
        tab->formats[p] = 0;
        p++;
    }
    nParams = p;

    if (dbg_times > 1) {
        gettimeofday(&tv2, NULL);
    }

    // executa o comando
    res =  PQexecPrepared(conn, stmt_name, nParams, tab->values, tab->lengths, tab->formats, 0);

    if (dbg_times > 1) {
        gettimeofday(&tv3, NULL);
        long tempo1 = ((tv3.tv_sec * 1000000) + tv3.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        long tempo2 = ((tv3.tv_sec * 1000000) + tv3.tv_usec) - ((tv2.tv_sec * 1000000) + tv2.tv_usec);
        fprintf(stderr, "%ld op_read_random [%s] tempo=%ld %ld\n", time(NULL), tab->name, tempo1, tempo2);
    }

    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0))  {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (dbg > 0) {
            fprintf(stderr, "%ld %d %s\n", time(NULL), PQresultStatus(res), PQerrorMessage(conn));
        }
    } else {
        if (!partial) {
            pq2cob(tab, res, fcd->record, reclen);
        }
        memcpy(fcd->status, ST_OK, 2);
    }
    PQclear(res);

    if (dbg > 0) {
        fprintf(stderr, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
    }
}
