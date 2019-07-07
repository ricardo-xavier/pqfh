#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

extern int pending_commits;

void op_delete(PGconn *conn, fcd_t *fcd) {

    unsigned int   fileid;
    unsigned short keylen; 
    table_t        *tab;
    column_t       *col;
    char           sql[4097], where[4097], kbuf[MAX_COL_LEN+1], stmt_name[65];
    int            p, nParams;
    list2_t        *ptr;
    PGresult       *res;

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_delete [%s]\n", tab->name);
    }

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_UPDEL, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return;
    }

    // verifica se o registro existe
    op_read_random(conn, fcd);
    if (memcmp(fcd->status, ST_OK, 2)) {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return;
    }

    strcpy(kbuf, getkbuf(fcd, 0, tab, &keylen));
    if (dbg > 1) {
        fprintf(stderr, "key %d %d [%s]\n", 0, keylen, kbuf);
    }
    sprintf(stmt_name, "%s_%ld_del", tab->name, tab->timestamp);

    // prepara o comando se ainda nao tiver preparado
    if (!tab->del_prepared) {

        sprintf(sql, "delete from %s.%s\n", tab->schema, tab->name);

        getwhere_prepared(tab, 0, where, 0, 'd');
        strcat(sql, "where ");
        strcat(sql, where);
        nParams = list2_size(tab->prms_delete);

        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }
        tab->del_prepared = true;

        res = PQprepare(conn, stmt_name, sql, nParams, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
    }

    // seta os parametros
    if (dbg > 2) {
        fprintf(stderr, "op_delete seta parametros para o delete\n");
    }
    p = 0;
    for (ptr=tab->prms_delete; ptr!=NULL; ptr=ptr->next) {
        col = (column_t *) ptr->buf;
        memcpy(tab->bufs[p], fcd->record+col->offset, col->len);
        tab->bufs[p][col->len] = 0;
        tab->values[p] = tab->bufs[p];
        tab->lengths[p] = col->len;
        tab->formats[p] = 0;
        if (dbg > 2) {
            fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, tab->bufs[p]);
        }
        p++;
    }
    nParams = p;

    // executa o comando
    if (dbg > 2) {
        fprintf(stderr, "op_delete executa o delete\n");
    }
    res =  PQexecPrepared(conn, stmt_name, nParams, tab->values, tab->lengths, tab->formats, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (dbg > 0) {
            fprintf(stderr, "%s\n", PQerrorMessage(conn));
        }
    } else {
        memcpy(fcd->status, ST_OK, 2);
    }
    PQclear(res);
    pending_commits++;

    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }

}
