#include "pqfh.h"

extern int dbg;

void deallocate(PGconn *conn, table_t *tab) {
    PGresult *res;
    char     sql[4097];
    int      k;

    for (k=0; k<MAX_KEYS; k++) {
        if (tab->read_prepared[k]) {
            tab->read_prepared[k] = false;
            sprintf(sql, "deallocate %s_%ld_%d", tab->name, tab->timestamp, k);
            if (dbg > 1) {
                fprintf(stderr, "%ld %s\n", time(NULL), sql);
            }
            res = PQexec(conn, sql);
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                fprintf(stderr, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
            }
            PQclear(res);
        }
    }

    if (tab->ins_prepared) {
        tab->ins_prepared = false;
        sprintf(sql, "deallocate %s_%ld_ins", tab->name, tab->timestamp);
        if (dbg > 1) {
            fprintf(stderr, "%ld %s\n", time(NULL), sql);
        }
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        }
        PQclear(res);
    }

    if (tab->upd_prepared) {
        tab->upd_prepared = false;
        sprintf(sql, "deallocate %s_%ld_upd", tab->name, tab->timestamp);
        if (dbg > 1) {
            fprintf(stderr, "%ld %s\n", time(NULL), sql);
        }
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        }
        PQclear(res);
    }

    if (tab->del_prepared) {
        tab->del_prepared = false;
        sprintf(sql, "deallocate %s_%ld_del", tab->name, tab->timestamp);
        if (dbg > 1) {
            fprintf(stderr, "%ld %s\n", time(NULL), sql);
        }
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        }
        PQclear(res);
    }
}

void close_cursor(PGconn *conn, table_t *tab) {
    PGresult *res;
    char     sql[4097];

    if (!tab->cursor) {
        return;
    }

    tab->cursor = false;
    sprintf(sql, "close cursor_%s_%ld", tab->name, tab->timestamp);
    if (dbg > 1) {
        fprintf(stderr, "%ld %s\n", time(NULL), sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
    }
    PQclear(res);
}
