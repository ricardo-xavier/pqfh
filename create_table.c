#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;

void  create_table(PGconn *conn, table_t *tab, fcd_t *fcd, unsigned short opcode) {
    char     sql[257], schema[33];
    char     indexdef[257], indexname[257];
    PGresult *res;
    char     *p, aux[257];
    int      len;

    strcpy(schema, get_schema(conn, tab->dictname));
    sprintf(sql, "create table %s.%s as select * from %s.%s where 1=2", schema, tab->name, schema, tab->dictname);
    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), sql);
        PQclear(res);
        return;
    }
    PQclear(res);

    sprintf(sql, "declare cursor_indexes cursor for\n  select indexname,indexdef from pg_indexes where tablename='%s'", tab->dictname);
    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), sql);
        PQclear(res);
        return;
    }
    PQclear(res);

    for (;;) {
        res = PQexec(conn, "fetch next in cursor_indexes");
        if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
            break;
        }
        strcpy(indexname, PQgetvalue(res, 0, 0));
        strcpy(indexdef, PQgetvalue(res, 0, 1));
        if (dbg > 1) {
            fprintf(stderr, "[%s] [%s]\n", indexname, indexdef);
        }
        len = strlen(indexname);
        if ((p = strstr(indexdef, indexname)) != NULL) {
            *p = 0;
            strcpy(aux, indexdef);
            strcat(aux, indexname);
            strcat(aux, "_");
            strcat(aux, tab->name);
            p += len;
            strcat(aux, p);
            strcpy(indexdef, aux);
        }
        len = strlen(tab->dictname);
        while ((p = strstr(indexdef, tab->dictname)) != NULL) {
            *p = 0;
            strcpy(aux, indexdef);
            strcat(aux, tab->name);
            p += len;
            strcat(aux, p);
            strcpy(indexdef, aux);
        }
        if (dbg > 1) {
            fprintf(stderr, "[%s]\n", indexdef);
        }
        res = PQexec(conn, indexdef);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            PQclear(res);
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), indexdef);
        }
        PQclear(res);
    }

    res = PQexec(conn, "CLOSE cursor_indexes");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), indexdef);
    }
    PQclear(res);

    res = PQexec(conn, "COMMIT");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n", PQerrorMessage(conn), indexdef);
    }
    PQclear(res);

    res = PQexec(conn, "BEGIN");
    PQclear(res);

    memcpy(fcd->status, ST_OK, 2);
}
