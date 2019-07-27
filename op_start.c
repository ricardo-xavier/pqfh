#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "pqfh.h"

extern int dbg;
extern int dbg_times;

bool partial=false;
bool eof_start=false;

void op_start(PGconn *conn, fcd_t *fcd, char *op) {

    unsigned int   fileid;
    unsigned short keyid, keylen;
    char           sql[4097], kbuf[MAX_KEY_LEN+1];
    table_t        *tab;
    PGresult       *res;
    char           where[4097], order[257];
    struct timeval tv1, tv2, tv3;

    if (dbg_times > 1) {
        gettimeofday(&tv1, NULL);
    }
    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "%ld op_start %s [%s]\n", time(NULL), op, tab->name);
    }

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_READ, 2);
        if (dbg > 0) {
            fprintf(stderr, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return;
    }

    eof_start = false;
    keyid = getshort(fcd->key_id);
    strcpy(kbuf, getkbuf(fcd, keyid, tab, &keylen));
    if (dbg > 1) {
        fprintf(stderr, "%ld key %d %d [%s]\n", time(NULL), keyid, keylen, kbuf);
    }

    // performance
    // se ja foi feito um read next com a mesma chave nao executa novamente
    if (dbg > 2) {
        fprintf(stderr, "%ld op_start verifica se foi feito um read next/prev com a mesma chave\n", time(NULL));
    }
    if (
            ((op[0] != '<') && (keyid == tab->key_next) && !memcmp(kbuf, tab->buf_next, keylen)) ||
            ((op[0] == '<') && (keyid == tab->key_prev) && !memcmp(kbuf, tab->buf_prev, keylen))) {
        if (op[1] == '=') {
            tab->restart = op[0] == '>' ? 'n' : 'p';
        }
        memcpy(fcd->status, ST_OK, 2);
        if (dbg > 0) {
            fprintf(stderr, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return;
    }

    // se o acesso for feito como entidade fraca faz um read com a chave parcial para ver se exxiste algum registro
    if (is_weak(tab->name)) {
        if (dbg > 2) {
            fprintf(stderr, "%ld op_start verifica se existe algum registro na tabela fraca\n", time(NULL));
        }
        partial = true;
        op_read_random(conn, fcd, false);
        if (memcmp(fcd->status, ST_OK, 2)) {
            memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
            eof_start = true;
            partial = false;
            if (dbg > 0) {
                fprintf(stderr, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
            }
            return;
        }
        partial = false;
    }

    close_cursor(conn, tab);
    tab->cursor = true;

    if (is_weak(tab->name)) {
        partial = true;
        getwhere(fcd->record, tab, keyid, "=", where, order);
        partial = false;
    } else {
        getwhere(fcd->record, tab, keyid, op, where, order);
    }
    sprintf(sql, "declare cursor_%s_%ld cursor with hold for\n  select * from %s.%s\n    where %s order by %s", 
        tab->name, tab->timestamp,  tab->schema, tab->name, where, order);

    if (dbg > 1) {
        fprintf(stderr, "%ld %s\n", time(NULL), sql);
    }

    if (dbg_times > 1) {
        gettimeofday(&tv2, NULL);
    }

    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        PQclear(res);
        exit(-1);
    }
    PQclear(res);

    if (dbg_times > 1) {
        gettimeofday(&tv3, NULL);
        long tempo1 = ((tv3.tv_sec * 1000000) + tv3.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        long tempo2 = ((tv3.tv_sec * 1000000) + tv3.tv_usec) - ((tv2.tv_sec * 1000000) + tv2.tv_usec);
        fprintf(stderr, "%ld op_start %s [%s] tempo=%ld %ld\n", time(NULL), op, tab->name, tempo1, tempo2);
    }

    if (op[0] != '<') {
        tab->key_next = keyid;
        op_next_prev(conn, fcd, 'p');
        tab->first = true;
    } else {
        tab->key_prev = keyid;
        op_next_prev(conn, fcd, 'n');
        tab->first = true;
    }
    if (!memcmp(fcd->status, ST_EOF, 2)) {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
    }
    //memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(stderr, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
    }
}
