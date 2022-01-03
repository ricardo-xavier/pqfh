#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;

#ifndef ISAM
void deadlock_log(char *msg) {
    char log[257], cmd[257];
    FILE *f;
    sprintf(log, "/tmp/deadlock_%ld.log", time(NULL));
    if ((f = fopen(log, "w")) == NULL) return;
    fprintf(flog, "%s\n", msg);
    fclose(f);
    sprintf(cmd, "ps aux >> %s", log);
    system(cmd);
}

void deallocate(PGconn *conn, table_t *tab) {
    PGresult *res;
    char     sql[MAX_REC_LEN+1];
    int      k;

    for (k=0; k<MAX_KEYS; k++) {
        if (tab->read_prepared[k]) {
            tab->read_prepared[k] = false;
            sprintf(sql, "deallocate %s_%s_%d", tab->name, tab->timestamp, k);
            if (dbg > 1 && log_table(tab->name)) {
                fprintf(flog, "%ld %s\n", time(NULL), sql);
            }
            res = PQexec(conn, sql);
            if (PQresultStatus(res) != PGRES_COMMAND_OK && log_table(tab->name)) {
                fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
            }
            PQclear(res);
        }
    }

    if (tab->ins_prepared) {
        tab->ins_prepared = false;
        sprintf(sql, "deallocate %s_%s_ins", tab->name, tab->timestamp);
        if (dbg > 1 && log_table(tab->name)) {
            fprintf(flog, "%ld %s\n", time(NULL), sql);
        }
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK && log_table(tab->name)) {
            fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        }
        PQclear(res);
    }

    if (tab->upd_prepared) {
        tab->upd_prepared = false;
        sprintf(sql, "deallocate %s_%s_upd", tab->name, tab->timestamp);
        if (dbg > 1 && log_table(tab->name)) {
            fprintf(flog, "%ld %s\n", time(NULL), sql);
        }
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK && log_table(tab->name)) {
            fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        }
        PQclear(res);
    }

    if (tab->del_prepared) {
        tab->del_prepared = false;
        sprintf(sql, "deallocate %s_%s_del", tab->name, tab->timestamp);
        if (dbg > 1 && log_table(tab->name)) {
            fprintf(flog, "%ld %s\n", time(NULL), sql);
        }
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK && log_table(tab->name)) {
            fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        }
        PQclear(res);
    }
}

void close_cursor(PGconn *conn, table_t *tab) {
    PGresult *res;
    char     sql[MAX_REC_LEN+1];

    if (!tab->cursor) {
        return;
    }

    tab->cursor = false;
    sprintf(sql, "close cursor_%s_%s", tab->name, tab->timestamp);
    if (dbg > 1 && log_table(tab->name)) {
        fprintf(flog, "%ld %s\n", time(NULL), sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK && log_table(tab->name)) {
        fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
    }
    PQclear(res);
}
#endif

extern int dbg_upd;

void dbg_record(fcd_t *fcd) {
    if (dbg_upd > 0 && log_fcd(fcd)) {
        char tmp[MAX_REC_LEN+1];
        short reclen;
        reclen = getshort(fcd->rec_len);
        memcpy(tmp, fcd->record, reclen);
        tmp[reclen] = 0;
        fprintf(flog, "[%s]\n", tmp);
    }
} 
