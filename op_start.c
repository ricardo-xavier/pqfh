#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

void op_start(PGconn *conn, fcd_t *fcd, char *op) {

    unsigned int   fileid;
    unsigned short keyid, keylen;
    char           sql[4097], kbuf[MAX_KEY_LEN+1];
    table_t        *tab;
    PGresult       *res;
    char           where[4097], order[257];

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_start %s [%s]\n", op, tab->name);
    }

    keyid = getshort(fcd->key_id);
    strcpy(kbuf, getkbuf(fcd, keyid, tab, &keylen));
    if (dbg > 1) {
        fprintf(stderr, "key %d %d [%s]\n", keyid, keylen, kbuf);
    }

    // performance
    // se ja foi feito um read next com a mesma chave nao executa novamente
    if (dbg > 2) {
        fprintf(stderr, "op_start verifica se foi feito um read next/prev com a mesma chave\n");
    }
    if (
            ((op[0] != '<') && (keyid == tab->key_next) && !memcmp(kbuf, tab->buf_next, keylen)) ||
            ((op[0] == '<') && (keyid == tab->key_prev) && !memcmp(kbuf, tab->buf_next, keylen))) {
        memcpy(fcd->status, ST_OK, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return;
    }

    close_cursor(conn, tab);
    tab->cursor = true;

    getwhere(fcd->record, tab, keyid, op, where, order);
    sprintf(sql, "declare cursor_%s_%ld cursor with hold for\n  select * from %s.%s\n    where %s order by %s", 
        tab->name, tab->timestamp,  get_schema(conn, tab->dictname), tab->name, where, order);

    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }

    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), sql);
        PQclear(res);
        exit(-1);
    }
    PQclear(res);

    if (op[0] != '<') {
        tab->key_next = keyid;
    } else {
        tab->key_prev = keyid;
    }
    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
}
