#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

void op_read_random(PGconn *conn, fcd_t *fcd) {

    unsigned int   fileid, keyoffset, keylen;
    unsigned short keyid, reclen;
    char           buf[257], sql[257], name[33];
    const char     *paramValues[1];
    int            paramFormats[1] = { 0 };
    int            paramLengths[1];
    table_t        *tab;
    column_t       *col;
    PGresult       *res;

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_read_random [%s]\n", tab->name);
    }

    kdb(fcd, &keyoffset, &keylen);
    col = get_col_at(tab, keyoffset);
    if (col == NULL) {
        fprintf(stderr, "coluna nao encontrada %d\n", keyoffset);
        exit(-1);
    }

    memcpy(buf, fcd->record+keyoffset, keylen);
    buf[keylen] = 0;

    keyid = getshort(fcd->key_id);
    if (dbg > 1) {
        fprintf(stderr, "key %d %d:%d [%s]\n", keyid, keyoffset, keylen, buf);
    }
    sprintf(name, "%s_%d", tab->name, keyid);

    // verifica se tem um read preparado com outra chave
    if (tab->read_prepared && (tab->key_read != keyid)) {
        fprintf(stderr, "read troca de chave nao implementada: %d\n", tab->key_read);
        exit(-1);
    }

    // prepara o comando se ainda nao tiver preparado
    if (!tab->read_prepared) {

        if (col->len == keylen) {
            sprintf(sql, "select * from %s.%s where %s = $1", get_schema(tab->name), tab->name, col->name);
        } else {
            sprintf(sql, "select * from %s.%s where k%d = $1", get_schema(tab->name), tab->name, keyid);
        }
        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }

        res = PQprepare(conn, name, sql, 1, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
                PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
        tab->read_prepared = true;
        tab->key_read = keyid;
    }

    // seta parametros
    paramValues[0] = buf;
    paramLengths[0] = keylen;

    // executa o comando
    res =  PQexecPrepared(conn, name, 1, paramValues, paramLengths, paramFormats, 0);
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
