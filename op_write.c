#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

extern int pending_commits;

void op_write(PGconn *conn, fcd_t *fcd) {

    unsigned short reclen, keyid; 
    table_t        *tab;
    char           name[33];
    unsigned int   fileid, offset;
    column_t       *col;
    char           sql[4097], aux[257];
    int            p, k;
    list2_t        *ptr;
    PGresult       *res;
    const char     *paramValues[256];
    int            paramFormats[256];
    int            paramLengths[256];

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_write [%s]\n", tab->name);
    }

    keyid = getshort(fcd->key_id);
    putshort(fcd->key_id, 0);

    // verifica se o registro ja existe
    op_read_random(conn, fcd);
    if (!memcmp(fcd->status, ST_OK, 2)) {
        memcpy(fcd->status, ST_DUPL_KEY, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return;
    }

    sprintf(name, "%s_ins", tab->name);

    // prepara o comando se ainda nao tiver preparado
    if (!tab->ins_prepared) {

        sprintf(sql, "insert into %s.%s (", get_schema(tab->name), tab->name);
        for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {
            col = (column_t *) ptr->buf;
            if (ptr != tab->columns) {
                strcat(sql, ",");
            }
            strcat(sql, col->name);
        }
        k = 0;
        for (ptr=tab->keys; ptr!=NULL; ptr=ptr->next) {
            _key_t *key = (_key_t *) ptr->buf;
            if (key->ncols > 1) {
                sprintf(aux, ",k%d", k);
                strcat(sql, aux);
            }
            k++;
        }
        
        strcat(sql, ")\nvalues (");
        p = 0;
        for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {
            col = (column_t *) ptr->buf;
            if (ptr != tab->columns) {
                strcat(sql, ",");
            }
            tab->bufs[p] = malloc(col->len+2);
            sprintf(aux, "$%d", ++p);
            strcat(sql, aux);
        }
        for (ptr=tab->keys; ptr!=NULL; ptr=ptr->next) {
            _key_t *key = (_key_t *) ptr->buf;
            if (key->ncols > 1) {
                tab->bufs[p] = malloc(key->len+2);
                sprintf(aux, ",$%d", ++p);
                strcat(sql, aux);
            }
        }
        strcat(sql, ")");

        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }
        tab->ins_prepared = true;

        res = PQprepare(conn, name, sql, 1, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
                PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
    }

    // seta os parametros
    p = 0;
    offset = 0;
    for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

        col = (column_t *) ptr->buf;

        paramValues[p] = tab->bufs[p];
        paramLengths[p] = col->len;
        paramFormats[p] = 0;

        if (col->tp == 'n') {
            if (col->dec > 0) {
                memcpy(tab->bufs[p], fcd->record + offset, col->len - col->dec);
                tab->bufs[p][col->len - col->dec] = '.';
                memcpy(tab->bufs[p] + col->len - col->dec + 1, fcd->record + offset + col->len - col->dec, col->dec);
                tab->bufs[p][col->len + 1] = 0;
            } else {
                memcpy(tab->bufs[p], fcd->record + offset, col->len);
                tab->bufs[p][col->len] = 0;
            }
            if ((fcd->record[offset + col->len - 1] & 0x40) == 0x40) {
                tab->bufs[p][0] = '-';
                if (col->dec > 0) {
                    tab->bufs[p][col->len] &= ~0x40;
                } else {
                    tab->bufs[p][col->len - 1] &= ~0x40;
                }
            }
        } else {
            memcpy(tab->bufs[p], fcd->record+offset, col->len);
            tab->bufs[p][col->len] = 0;
        }

        if (dbg > 2) {
            fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, offset, col->len, col->dec, tab->bufs[p]);
        }
        p++;
        offset += col->len;

    }

    // monta chaves compostas
    for (ptr=tab->keys, k=0; ptr!=NULL; ptr=ptr->next, k++) {
        _key_t *key = (_key_t *) ptr->buf;
        if (key->ncols > 1) {
            paramValues[p] = tab->bufs[p];
            paramLengths[p] = col->len;
            paramFormats[p] = 0;
            column_t *col;
            int c;
            offset = 0;
            for (c=0; c<key->ncols; c++) {
                col = key->columns[c];
                memcpy(tab->bufs[p]+offset, fcd->record+col->offset, col->len);
                offset += col->len;
            }
            tab->bufs[p][offset] = 0;
            if (dbg > 2) {
                fprintf(stderr, "    %d k%d %d [%s]\n", p, k, key->len, tab->bufs[p]);
            }
            p++;
        }
    }

    // executa o comando
    res =  PQexecPrepared(conn, name, p, paramValues, paramLengths, paramFormats, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        memcpy(fcd->status, ST_ERROR, 2);
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
