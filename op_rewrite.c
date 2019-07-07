#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern int pending_commits;
extern char backup[MAX_REC_LEN+1];

bool op_rewrite(PGconn *conn, fcd_t *fcd) {

    unsigned int   fileid;
    unsigned short reclen, keyid, keylen; 
    table_t        *tab;
    column_t       *col;
    char           record[MAX_REC_LEN+1], kbuf[MAX_KEY_LEN+1], sql[4097], aux[257];
    int            p, nParams;
    list2_t        *ptr;
    PGresult       *res;
    char           where[4097], stmt_name[65];

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_rewrite [%s]\n", tab->name);
    }

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_UPDEL, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return false;
    }

    keyid = getshort(fcd->key_id);
    putshort(fcd->key_id, 0);
    memcpy(record, fcd->record, reclen);

    // performance
    // verifica se o registro mudou antes de fazer o update
    if (dbg > 2) {
        fprintf(stderr, "op_rewrite verifica se o registro existe\n");
    }
    op_read_random(conn, fcd);
    if (memcmp(fcd->status, ST_OK, 2)) {
        // registro nao encontrado
        putshort(fcd->key_id, keyid);
        return false;
    }
    if (dbg > 2) {
        fprintf(stderr, "op_rewrite verifica se o registro foi alterado\n");
    }
    if (!memcmp(record, fcd->record, reclen)) {
        memcpy(fcd->status, ST_OK, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return false;
    }

    memcpy(backup, fcd->record, reclen);
    memcpy(fcd->record, record, reclen);

    strcpy(kbuf, getkbuf(fcd, 0, tab, &keylen));
    if (dbg > 1) {
        fprintf(stderr, "key %d %d [%s]\n", 0, keylen, kbuf);
    }
    sprintf(stmt_name, "%s_%ld_upd", tab->name, tab->timestamp);

    // prepara o comando se ainda nao tiver preparado
    if (!tab->upd_prepared) {

        sprintf(sql, "update %s.%s\n", tab->schema, tab->name);

        p = 0;
        for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

            col = (column_t *) ptr->buf;

            // ignora pk
            if (col->pk) {
                continue;
            }

            col->p = p;
            if (p == 0) {
                strcat(sql, "  set ");
            } else {
                strcat(sql, "     ,");
            }
            sprintf(aux, "%s=$%d\n", col->name, ++p);
            strcat(sql, aux);
        }

        getwhere_prepared(tab, keyid, where, p, 'u');
        strcat(sql, "where ");
        strcat(sql, where);
        p += list2_size(tab->prms_rewrite);

        nParams = p;
        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }
        tab->upd_prepared = true;

        res = PQprepare(conn, stmt_name, sql, nParams, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
    }

    // seta os parametros fora da pk
    if (dbg > 2) {
        fprintf(stderr, "op_rewrite seta parametros para o update\n");
    }
    p = 0;
    for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

        col = (column_t *) ptr->buf;

        // ignora pk
        if (col->pk) {
            continue;
        }

        tab->values[p] = tab->bufs[p];
        tab->lengths[p] = col->len;
        tab->formats[p] = 0;

        if (col->tp == 'n') {
            if (col->dec > 0) {
                memcpy(tab->bufs[p], fcd->record + col->offset, col->len - col->dec);
                tab->bufs[p][col->len - col->dec] = '.';
                memcpy(tab->bufs[p] + col->len - col->dec + 1, fcd->record + col->offset + col->len - col->dec, col->dec);
                tab->bufs[p][col->len + 1] = 0;
            } else {
                memcpy(tab->bufs[p], fcd->record + col->offset, col->len);
                tab->bufs[p][col->len] = 0;
            }
            if ((fcd->record[col->offset + col->len - 1] & 0x40) == 0x40) {
                tab->bufs[p][0] = '-';
                if (col->dec > 0) {
                    tab->bufs[p][col->len] &= ~0x40;
                } else {
                    tab->bufs[p][col->len - 1] &= ~0x40;
                }
            }
        } else {
            memcpy(tab->bufs[p], fcd->record+col->offset, col->len);
            tab->bufs[p][col->len] = 0;
        }

        if (dbg > 2) {
            fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, tab->bufs[p]);
        }
        p++;

    }

    // pk
    for (ptr=tab->prms_rewrite; ptr!=NULL; ptr=ptr->next) {
        col = (column_t *) ptr->buf;
        tab->values[p] = tab->bufs[p];
        tab->lengths[p] = col->len;
        tab->formats[p] = 0;
        memcpy(tab->bufs[p], fcd->record+col->offset, col->len);
        tab->bufs[p][col->len] = 0;

        if (dbg > 2) {
            fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, tab->bufs[p]);
        }
        p++;
    }

    nParams = p;

    // executa o comando
    if (dbg > 2) {
        fprintf(stderr, "op_rewrite executa o update\n");
    }
    res =  PQexecPrepared(conn, stmt_name, nParams, tab->values, tab->lengths, tab->formats, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (dbg > 0) {
            fprintf(stderr, "%s\n", PQerrorMessage(conn));
        }
    } else {
        if (tab->clones != NULL) {
            replica_rewrite(tab);
        }
        memcpy(fcd->status, ST_OK, 2);
    }
    PQclear(res);
    pending_commits++;

    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
    putshort(fcd->key_id, keyid);
    return true;

}
