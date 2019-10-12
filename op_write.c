#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern int dbg_upd;
extern bool executed;

extern int pending_commits;

bool op_write(PGconn *conn, fcd_t *fcd) {
#ifndef ISAM
    rnsigned int   fileid;
    table_t        *tab;
    column_t       *col;
    char           stmt_name[65], prefixo[7];
    char           sql[4097], aux[257];
    unsigned short keyid; 
    int            p;
    list2_t        *ptr;
    PGresult       *res;
    short          op;
#endif    

    if (!memcmp(fcd->file_name, "pqfh", 4)) {
        command(conn, fcd);
        return true;
    }

#ifndef ISAM
    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_WRITE, 2);
        if (dbg > 0) {
            short fnlen = getshort(fcd->file_name_len);
            char filename[257];
            memcpy(filename, (char *) fcd->file_name, fnlen);
            filename[fnlen] = 0;
            fprintf(flog, "%ld op_write [%s] %d\n", time(NULL), filename, (int) fcd->open_mode);
            fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return false;
    }

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    op = OP_WRITE;
    if (dbg > 0 || DBG_UPD) {
        fprintf(flog, "%ld op_write [%s]\n", time(NULL), tab->name);
        dbg_record(fcd);
    }
#endif

#ifndef ISAM
    keyid = getshort(fcd->key_id);
    putshort(fcd->key_id, 0);

    // verifica se o registro ja existe
    if (dbg > 2) {
        fprintf(flog, "%ld op_write verifica se o registro existe no banco\n", time(NULL));
    }
    op_read_random(conn, fcd, false);
    if (!memcmp(fcd->status, ST_OK, 2)) {
        memcpy(fcd->status, ST_DUPL_KEY, 2);
        if (dbg > 0 || DBG_UPD) {
            fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        putshort(fcd->key_id, keyid);
        return false;
    }

    sprintf(stmt_name, "%s_%ld_ins", tab->name, tab->timestamp);

    // prepara o comando se ainda nao tiver preparado
    if (!tab->ins_prepared) {

        sprintf(sql, "insert into %s.%s (", tab->schema, tab->name);
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
            fprintf(flog, "%ld %s\n", time(NULL), sql);
        }
        tab->ins_prepared = true;

        res = PQprepare(conn, stmt_name, sql, p, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);

    }

    if (dbg > 2) {
        fprintf(flog, "%ld op_write seta parametros para o insert\n", time(NULL));
    }
    // seta os parametros
    p = 0;
    for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

        col = (column_t *) ptr->buf;
        col->p = p;

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
            if (!tab->bufs[p][0]) {
                strcpy(tab->bufs[p], "0");
            }
        } else {
            memcpy(tab->bufs[p], fcd->record+col->offset, col->len);
            tab->bufs[p][col->len] = 0;
        }

        if (dbg > 2) {
            fprintf(flog, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, tab->bufs[p]);
        }
        p++;

    }

    // executa o comando
    if (dbg > 2) {
        fprintf(flog, "%ld op_write executa o insert\n", time(NULL));
    }
    res =  PQexecPrepared(conn, stmt_name, p, tab->values, tab->lengths, tab->formats, 0);
    executed = true;
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        memcpy(fcd->status, ST_ERROR, 2);
        if (strstr(PQerrorMessage(conn), "deadlock")) {
            deadlock_log(PQerrorMessage(conn));
        }
        if (dbg > 0) {
            fprintf(flog, "%ld %s\n", time(NULL), PQerrorMessage(conn));
        }
    } else {
        if (tab->clones != NULL) {
            replica_write(tab);
        }
        memcpy(fcd->status, ST_OK, 2);
    }
    PQclear(res);
    pending_commits++;

    if (dbg > 0 || DBG_UPD) {
        fprintf(flog, "%ld status=%c%c commits=%d\n\n", time(NULL), fcd->status[0], fcd->status[1], pending_commits);
    }
    putshort(fcd->key_id, keyid);

#ifdef API
    if (tab->api[0] && !memcmp(fcd->status, ST_OK, 2)) {
        thread_api_start('i', tab, fcd);
    }
#endif
#endif
    return false;

}
