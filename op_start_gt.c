#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

void op_start_gt(PGconn *conn, fcd_t *fcd) {

    unsigned int   fileid, keyoffset, keylen;
    unsigned short keyid;
    char           buf[4097], sql[257];
    table_t        *tab;
    PGresult       *res;
    column_t       *col;

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_start_gt [%s]\n", tab->name);
    }

    kdb(fcd, &keyoffset, &keylen);

    col = get_col_at(tab, keyoffset);
    if (col == NULL) {
        fprintf(stderr, "coluna nao encontrada %d\n", keyoffset);
        exit(-1);
    }

    keyid = getshort(fcd->key_id);
    // performance
    // se ja foi feito um read next com a mesma chave nao executa novamente
    if ((keyid == tab->key_start) && !memcmp(fcd->record+keyoffset, tab->buf_start, keylen)) {
        memcpy(fcd->status, ST_OK, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return;
    }

    memcpy(buf, fcd->record+keyoffset, keylen);
    buf[keylen] = 0;
    if (dbg > 1) {
        fprintf(stderr, "key %d %d:%d [%s]\n", keyid, keyoffset, keylen, buf);
    }

    if (tab->key_start != -1) {
        sprintf(sql, "close cursor_%s", tab->name);
        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
                PQerrorMessage(conn), sql);
            PQclear(res);
            exit(-1);
        }
        PQclear(res);
    }

    if (col->len == keylen) {
        sprintf(sql, "declare cursor_%s cursor with hold for\n  select * from %s.%s\n    where %s > %s%s%s order by %s", 
            tab->name, get_schema(tab->name), tab->name, col->name, 
            col->tp == 's' ? "'" : "",
            buf, 
            col->tp == 's' ? "'" : "",
            col->name);
    } else {
        sprintf(sql, "declare cursor_%s cursor with hold for\n  select * from %s.%s\n    where k%d > '%s' order by k%d", 
            tab->name, get_schema(tab->name), tab->name, keyid, buf, keyid);
    }
    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }

    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
            PQerrorMessage(conn), sql);
        PQclear(res);
        exit(-1);
    }
    PQclear(res);

    tab->key_start = keyid;
    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
}
