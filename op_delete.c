#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

extern int pending_commits;
extern bool chaves_concatenadas;

void op_delete(PGconn *conn, fcd_t *fcd) {

    unsigned int   fileid, keylen, keyoffset, remainder, offset;
    unsigned short reclen, keyid; 
    table_t        *tab;
    column_t       *pk, *col;
    char           buf[257], sql[4097], aux[257], bufs[16][257];
    int            p, nParams;
    list2_t        *ptr;
    char           name[33];
    PGresult       *res;
    const char     *paramValues[256];
    int            paramFormats[256];
    int            paramLengths[256];

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_delete [%s]\n", tab->name);
    }

    keyid = getshort(fcd->key_id);
    putshort(fcd->key_id, 0);

    // verifica se o registro existe
    op_read_random(conn, fcd);
    if (memcmp(fcd->status, ST_OK, 2)) {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return;
    }

    kdb(fcd, &keyoffset, &keylen);
    pk = get_col_at(tab, keyoffset);
    if (pk == NULL) {
        fprintf(stderr, "coluna nao encontrada %d\n", keyoffset);
        exit(-1);
    }
    if (pk->len == keylen) {
        fprintf(stderr, "delete com chave simples nao implementado\n");
        exit(-1);
    }
    if (keyoffset != 0) {
        fprintf(stderr, "delete com keyoffset != 0 nao implementado\n");
        exit(-1);
    }

    memcpy(buf, fcd->record+keyoffset, keylen);
    buf[keylen] = 0;
    if (dbg > 1) {
        fprintf(stderr, "key %d %d:%d [%s]\n", 0, keyoffset, keylen, buf);
    }
    sprintf(name, "%s_del", tab->name);

    // prepara o comando se ainda nao tiver preparado
    if (!tab->del_prepared) {

        sprintf(sql, "delete from %s.%s\n", get_schema(conn, tab->name), tab->name);

        if (!chaves_concatenadas) {
            char where[4097];
            getwhere_prepared(tab, keyid, where, 0, 'd');
            strcat(sql, "where ");
            strcat(sql, where);
            nParams = list2_size(tab->prms_delete);

        } else {

            // monta o where com as colunas da pk
            p = 0;
            remainder = keylen;
            for (ptr=tab->columns; (remainder > 0) && (ptr != NULL); ptr=ptr->next) {

                col = (column_t *) ptr->buf;

                if (remainder == keylen) {
                    strcat(sql, "  where ");
                } else {
                    strcat(sql, "    and ");
                }
                tab->bufs[p] = malloc(col->len+2);
                sprintf(aux, "%s = $%d\n", col->name, ++p);
                strcat(sql, aux);
    
                remainder -= col->len;
            }
            nParams = p;
        }

        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }
        tab->del_prepared = true;

        res = PQprepare(conn, name, sql, nParams, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
                PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
    }

    // seta os parametros
    p = 0;
    if (!chaves_concatenadas) {
        list2_t *ptr;
        column_t *col;
        int seq=0;

        for (ptr=tab->prms_delete; ptr!=NULL; ptr=ptr->next) {
            col = (column_t *) ptr->buf;
            memcpy(bufs[seq], fcd->record+col->offset, col->len);
            bufs[seq][col->len] = 0;
            paramValues[seq] = bufs[seq];
            paramLengths[seq] = col->len;
            paramFormats[seq] = 0;
            if (dbg > 2) {
                fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, bufs[seq]);
            }
            seq++;
            p++;
        }
    } else {

        remainder = keylen;
        offset = 0;
        for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

            col = (column_t *) ptr->buf;

            // pk
            if (remainder > 0) {

                paramValues[p] = tab->bufs[p];
                paramLengths[p] = col->len;
                paramFormats[p] = 0;

                remainder -= col->len;
                memcpy(tab->bufs[p], fcd->record+offset, col->len);
                tab->bufs[p][col->len] = 0;

                if (dbg > 2) {
                    fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, offset, col->len, col->dec, tab->bufs[p]);
                }

                p++;
                offset += col->len;
            }
        }
    }
    nParams = p;

    // executa o comando
    res =  PQexecPrepared(conn, name, nParams, paramValues, paramLengths, paramFormats, 0);
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
    putshort(fcd->key_id, keyid);

}
