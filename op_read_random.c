#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

void op_read_random(PGconn *conn, fcd_t *fcd) {

    unsigned int   fileid ;
    unsigned short keyid, reclen, keylen;
    char           kbuf[MAX_KEY_LEN+1], sql[257], stmt_name[65], where[4097];
    int            nParams, p;
    table_t        *tab;
    column_t       *col;
    PGresult       *res;
    list2_t        *ptr;

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_read_random [%s]\n", tab->name);
    }

    keyid = getshort(fcd->key_id);
    strcpy(kbuf, getkbuf(fcd, keyid, tab, &keylen));
    if (dbg > 1) {
        fprintf(stderr, "key %d %d [%s]\n", keyid, keylen, kbuf);
    }
    sprintf(stmt_name, "%s_%ld_%d", tab->name, tab->timestamp, keyid);

    // verifica se tem um read preparado com outra chave
    if (tab->read_prepared && (tab->key_read != keyid)) {
        fprintf(stderr, "read troca de chave nao implementada: %d\n", tab->key_read);
        exit(-1);
    }

    // prepara o comando se ainda nao tiver preparado
    if (!tab->read_prepared) {

        getwhere_prepared(tab, keyid, where, 0, 's');
        sprintf(sql, "select * from %s.%s where %s", tab->schema, tab->name, where);
        nParams = list2_size(tab->prms);

        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }

        res = PQprepare(conn, stmt_name, sql, nParams, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
        tab->read_prepared = true;
        tab->key_read = keyid;
    }

    // seta parametros
    p = 0;
    for (ptr=tab->prms; ptr!=NULL; ptr=ptr->next) {
        col = (column_t *) ptr->buf;
        memcpy(tab->bufs[p], fcd->record+col->offset, col->len);
        tab->bufs[p][col->len] = 0;
        tab->values[p] = tab->bufs[p];
        tab->lengths[p] = col->len;
        tab->formats[p] = 0;
        p++;
    }
    nParams = p;

    // executa o comando
    res =  PQexecPrepared(conn, stmt_name, nParams, tab->values, tab->lengths, tab->formats, 0);
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0))  {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (dbg > 0) {
            fprintf(stderr, "%s\n", PQerrorMessage(conn));
        }
    } else {
        pq2cob(tab, res, fcd->record, reclen);
        memcpy(fcd->status, ST_OK, 2);
    }
    PQclear(res);

    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
}
