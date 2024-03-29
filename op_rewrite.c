#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "pqfh.h"

extern int dbg;
extern int dbg_upd;
extern int dbg_times;
extern int pending_commits;
extern bool executed;
extern bool fatal;

extern char kbuf_read[MAX_KEY_LEN+1];

bool op_rewrite(PGconn *conn, fcd_t *fcd) {
    funcao = _OP_REWRITE;    

    unsigned int   fileid;
    unsigned short keyid, keylen; 
    table_t        *tab;
    column_t       *col;
    char           record[MAX_REC_LEN+1];
    unsigned short reclen;
    char           kbuf[MAX_KEY_LEN+1], sql[MAX_REC_LEN+1], aux[257];
    int            p, nParams;
    list2_t        *ptr;
    PGresult       *res;
    char           where[MAX_REC_LEN+1], stmt_name[65];
    short          op;
    struct timeval tv1, tv2, tv3;

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_UPDEL, 2);
        if (dbg > 0) {
            short fnlen = getshort(fcd->file_name_len);
            char filename[257];
            memcpy(filename, (char *) fcd->file_name, fnlen);
            filename[fnlen] = 0;
            if (log_table(filename)) {
                fprintf(flog, "%ld op_rewrite [%s] %d\n", time(NULL), filename, (int) fcd->open_mode);
                fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
            }    
        }
        return false;
    }

    if (dbg_times > 1) {
        gettimeofday(&tv1, NULL);
    }

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    op = OP_REWRITE;
    if (dbg > 0 || DBG_UPD) {
        if (log_table(tab->name)) fprintf(flog, "%ld op_rewrite [%s]\n", time(NULL), tab->name);
        dbg_record(fcd);
    }
    tab->restart = 0;
    tab->key_next = -1;
    tab->key_prev = -1;

    reclen = getshort(fcd->rec_len);
    memcpy(record, fcd->record, reclen);
    keyid = getshort(fcd->key_id);
    putshort(fcd->key_id, 0);

    // performance
    // verifica se o registro mudou antes de fazer o update
    if (dbg > 2) {
        if (log_table(tab->name)) fprintf(flog, "%ld op_rewrite verifica se o registro existe no banco\n", time(NULL));
    }
    op_read_random(conn, fcd, false);
    if (memcmp(fcd->status, ST_OK, 2)) {
        // registro nao encontrado
        putshort(fcd->key_id, keyid);
        warningbd("rewrite", tab->name, kbuf_read, fcd->status);
        return false;
    }
    if (dbg > 2) {
        if (log_table(tab->name)) fprintf(flog, "%ld op_rewrite verifica se o registro foi alterado\n", time(NULL));
    }
    if (!memcmp(record, fcd->record, reclen)) {
        memcpy(fcd->status, ST_OK, 2);
        putshort(fcd->key_id, keyid);
        if (dbg > 0 || DBG_UPD) {
            if (log_table(tab->name)) fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return true;
    }

    memcpy(fcd->record, record, reclen);

    strcpy(kbuf, getkbuf(fcd, 0, tab, &keylen));
    if (dbg > 1) {
        if (log_table(tab->name)) fprintf(flog, "%ld key %d %d [%s]\n", time(NULL), 0, keylen, kbuf);
    }
    sprintf(stmt_name, "%s_%s_upd", tab->name, tab->timestamp);

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

        getwhere_prepared(tab, 0, where, p, 'u');
        strcat(sql, "where ");
        strcat(sql, where);
        p += list2_size(tab->prms_rewrite);

        nParams = p;
        if (dbg > 1) {
            if (log_table(tab->name)) fprintf(flog, "%ld %s\n", time(NULL), sql);
        }
        tab->upd_prepared = true;

        res = PQprepare(conn, stmt_name, sql, nParams, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            if (log_table(tab->name)) fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
    }

    // seta os parametros fora da pk
    if (dbg > 2) {
        if (log_table(tab->name)) fprintf(flog, "%ld op_rewrite seta parametros para o update\n", time(NULL));
    }
    p = 0;
    fatal = false;
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
                tab->bufs[p][0] = col->len > 1 ? '-' : '0';
                if (col->dec > 0) {
                    tab->bufs[p][col->len] &= ~0x40;
                } else {
                    tab->bufs[p][col->len - 1] &= ~0x40;
                }
            }
            valida_numero(tab, col->name, tab->bufs[p], col->dec > 0);
        } else {
            memcpy(tab->bufs[p], fcd->record+col->offset, col->len);
            tab->bufs[p][col->len] = 0;
        }

        if (dbg > 2) {
            if (log_table(tab->name)) fprintf(flog, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, tab->bufs[p]);
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

        if (col->tp == 'n') {
            valida_numero(tab, col->name, tab->bufs[p], col->dec > 0);
        }

        if (dbg > 2) {
            if (log_table(tab->name)) fprintf(flog, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, tab->bufs[p]);
        }
        p++;
    }
    valida_comando("REWRITE", tab->name);

    nParams = p;

    if (dbg_times > 1) {
        gettimeofday(&tv2, NULL);
    }

    // executa o comando
    if (dbg > 2) {
        if (log_table(tab->name)) fprintf(flog, "%ld op_rewrite executa o update\n", time(NULL));
    }
    res =  PQexecPrepared(conn, stmt_name, nParams, tab->values, tab->lengths, tab->formats, 0);
    executed = true;

    if (dbg_times > 1) {
        gettimeofday(&tv3, NULL);
        long tempo1 = ((tv3.tv_sec * 1000000) + tv3.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        long tempo2 = ((tv3.tv_sec * 1000000) + tv3.tv_usec) - ((tv2.tv_sec * 1000000) + tv2.tv_usec);
        if (log_table(tab->name)) fprintf(flog, "%ld op_rewrite [%s] tempo=%ld %ld\n", time(NULL), tab->name, tempo1, tempo2);
    }

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        errorbd(stmt_name, res);    
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (strstr(PQerrorMessage(conn), "deadlock")) {
            deadlock_log(PQerrorMessage(conn));
        }
        if (dbg > 0) {
            if (log_table(tab->name)) fprintf(flog, "%ld %s\n", time(NULL), PQerrorMessage(conn));
        }
    } else {
        if (tab->clones != NULL) {
            replica_rewrite(tab);
        }
        memcpy(fcd->status, ST_OK, 2);
    }
    PQclear(res);
    pending_commits++;

    if (dbg > 0 || DBG_UPD) {
        if (log_table(tab->name)) fprintf(flog, "%ld status=%c%c commits=%d\n\n", time(NULL), fcd->status[0], fcd->status[1], pending_commits);
    }
    putshort(fcd->key_id, keyid);

#ifdef API
    if (tab->api[0] && !memcmp(fcd->status, ST_OK, 2)) {
        thread_api_start('u', tab, fcd);
    }
#endif
    return true;

}
