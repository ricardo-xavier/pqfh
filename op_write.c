#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

extern int pending_commits;

void op_write(PGconn *conn, fcd_t *fcd) {

    unsigned short reclen, keyid; 
    table_t        *tab;
    char           stmt_name[65], prefixo[7];
    unsigned int   fileid;
    column_t       *col;
    char           sql[4097], aux[257];
    int            p;
    list2_t        *ptr;
    PGresult       *res;

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_write [%s]\n", tab->name);
    }

    keyid = getshort(fcd->key_id);
    putshort(fcd->key_id, 0);

    // verifica se o registro ja existe
    if (dbg > 2) {
        fprintf(stderr, "op_write verifica se o registro existe\n");
    }
    op_read_random(conn, fcd);
    if (!memcmp(fcd->status, ST_OK, 2)) {
        memcpy(fcd->status, ST_DUPL_KEY, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        putshort(fcd->key_id, keyid);
        return;
    }

    sprintf(stmt_name, "%s_%ld_ins", tab->name, tab->timestamp);

    // prepara o comando se ainda nao tiver preparado
    if (!tab->ins_prepared) {

        sprintf(sql, "insert into %s.%s (", get_schema(conn, tab->dictname), tab->name);
        for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {
            col = (column_t *) ptr->buf;
            if (ptr == tab->columns) {
                memcpy(prefixo, col->name, 6);
                prefixo[6] = 0;
            }
            if (ptr != tab->columns) {
                strcat(sql, ",");
            }
            strcat(sql, col->name);
        }
        
        strcat(sql, ")\nvalues (");
        p = 0;
        for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {
            col = (column_t *) ptr->buf;
            if (ptr != tab->columns) {
                strcat(sql, ",");
            }
            sprintf(aux, "$%d", ++p);
            strcat(sql, aux);
        }
        strcat(sql, ")");

        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }
        tab->ins_prepared = true;

        res = PQprepare(conn, stmt_name, sql, p, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
    }

    if (dbg > 2) {
        fprintf(stderr, "op_write seta parametros para o insert\n");
    }
    // seta os parametros
    p = 0;
    for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

        col = (column_t *) ptr->buf;

        if (ptr == tab->columns) {
            memcpy(prefixo, col->name, 6);
            prefixo[6] = 0;
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

    // executa o comando
    if (dbg > 2) {
        fprintf(stderr, "op_write executa o insert\n");
    }
    res =  PQexecPrepared(conn, stmt_name, p, tab->values, tab->lengths, tab->formats, 0);
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