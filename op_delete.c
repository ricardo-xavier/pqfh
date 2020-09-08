#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern int dbg_upd;
extern bool executed;
extern bool fatal;

extern int pending_commits;

void op_delete(PGconn *conn, fcd_t *fcd) {
    funcao = _OP_DELETE;    

    unsigned int   fileid;
    unsigned short keylen; 
    table_t        *tab;
    column_t       *col;
    char           sql[MAX_REC_LEN+1], where[MAX_REC_LEN+1], kbuf[MAX_COL_LEN+1], stmt_name[65];
    int            p, nParams;
    list2_t        *ptr;
    PGresult       *res;
    short          op;

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_UPDEL, 2);
        if (dbg > 0) {
            short fnlen = getshort(fcd->file_name_len);
            char filename[257];
            memcpy(filename, (char *) fcd->file_name, fnlen);
            filename[fnlen] = 0;
            fprintf(flog, "%ld op_delete [%s] %d\n", time(NULL), filename, (int) fcd->open_mode);
            fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return;
    }

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    op = OP_DELETE;
    if (dbg > 0 || DBG_UPD) {
        fprintf(flog, "%ld op_delete [%s]\n", time(NULL), tab->name);
        dbg_record(fcd);
    }
    tab->restart = 0;
    tab->key_next = -1;
    tab->key_prev = -1;

    // verifica se o registro existe
    op_read_random(conn, fcd, false);
    if (memcmp(fcd->status, ST_OK, 2)) {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (dbg > 0 || DBG_UPD) {
            fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return;
    }

    strcpy(kbuf, getkbuf(fcd, 0, tab, &keylen));
    if (dbg > 1) {
        fprintf(flog, "%ld key %d %d [%s]\n", time(NULL), 0, keylen, kbuf);
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
            fprintf(flog, "%ld %s\n", time(NULL), sql);
        }
        tab->del_prepared = true;

        res = PQprepare(conn, stmt_name, sql, nParams, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
    }

    // seta os parametros
    if (dbg > 2) {
        fprintf(flog, "%ld op_delete seta parametros para o delete\n", time(NULL));
    }
    p = 0;
    fatal = false;
    for (ptr=tab->prms_delete; ptr!=NULL; ptr=ptr->next) {
        col = (column_t *) ptr->buf;
        col->p = p;
        memcpy(tab->bufs[p], fcd->record+col->offset, col->len);
        tab->bufs[p][col->len] = 0;
        tab->values[p] = tab->bufs[p];
        tab->lengths[p] = col->len;
        tab->formats[p] = 0;
        if (col->tp == 'n') {
            valida_numero(tab, col->name, tab->bufs[p], col->dec > 0);
        }
        if (dbg > 2) {
            fprintf(flog, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, tab->bufs[p]);
        }
        p++;
    }
    valida_comando("DELETE", tab->name);
    nParams = p;

    // executa o comando
    if (dbg > 2) {
        fprintf(flog, "%ld op_delete executa o delete\n", time(NULL));
    }
    res =  PQexecPrepared(conn, stmt_name, nParams, tab->values, tab->lengths, tab->formats, 0);
    executed = true;
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        errorbd(stmt_name, res);    
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (strstr(PQerrorMessage(conn), "deadlock")) {
            deadlock_log(PQerrorMessage(conn));
        }
        if (dbg > 0) {
            fprintf(flog, "%ld %s\n", time(NULL), PQerrorMessage(conn));
        }
    } else {
        if (tab->clones != NULL) {
            replica_delete(tab);
        }
        memcpy(fcd->status, ST_OK, 2);
    }
    PQclear(res);
    pending_commits++;

    if (dbg > 0 || DBG_UPD) {
        fprintf(flog, "%ld status=%c%c commits=%d\n\n", time(NULL), fcd->status[0], fcd->status[1], pending_commits);
    }
#ifdef API
    if (tab->api[0] && !memcmp(fcd->status, ST_OK, 2)) {
        thread_api_start('d', tab, fcd);
    }
#endif

}
