#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern bool chaves_concatenadas;

void op_start(PGconn *conn, fcd_t *fcd, char *op) {

    unsigned int   fileid, keyoffset, keylen;
    unsigned short keyid;
    char           buf[4097], sql[4097], kname[33];
    table_t        *tab;
    PGresult       *res;
    column_t       *col;

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_start %s [%s]\n", op, tab->name);
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
    if (
            ((op[0] == '>') && (keyid == tab->key_next) && !memcmp(fcd->record+keyoffset, tab->buf_next, keylen)) ||
            ((op[0] == '<') && (keyid == tab->key_prev) && !memcmp(fcd->record+keyoffset, tab->buf_next, keylen))) {
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

    if (((op[0] == '>') && (tab->key_next != -1)) ||
        ((op[0] == '<') && (tab->key_prev != -1))) {
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

    if (!chaves_concatenadas) {
        char where[4097], order[257];
        getwhere(fcd->record, tab, keyid, op, where, order);
        sprintf(sql, "declare cursor_%s cursor with hold for\n  select * from %s.%s\n    where %s order by %s", 
            tab->name, get_schema(conn, tab->name), tab->name, where, order);
    } else {

        if (col->len == keylen) {
            sprintf(sql, "declare cursor_%s cursor with hold for\n  select * from %s.%s\n    where %s %s %s%s%s order by %s", 
                tab->name, get_schema(conn, tab->name), tab->name, col->name, op,
                col->tp == 's' ? "'" : "",
                buf, 
                col->tp == 's' ? "'" : "",
                col->name);
        } else {

            memcpy(kname, col->name, 6);
            sprintf(kname+6, "key%d", keyid);
            sprintf(sql, "declare cursor_%s cursor with hold for\n  select * from %s.%s\n    where %s %s '%s' order by %s", 
                tab->name, get_schema(conn, tab->name), tab->name, kname, op, buf, kname);
        }
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

    if (op[0] == '>') {
        tab->key_next = keyid;
    } else {
        tab->key_prev = keyid;
    }
    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
}
