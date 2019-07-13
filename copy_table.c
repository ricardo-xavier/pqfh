#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;
extern int pending_commits;

void  copy_table(PGconn *conn, char *source, char *dest) {
    char     sql[257], schema[33];
    PGresult *res;

    strcpy(schema, get_schema(conn, source));
    sprintf(sql, "create table %s.%s as select * from %s.%s", schema, dest, schema, source);
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
    pending_commits++;
    commit();
}

void  truncate_table(PGconn *conn, char *tabname) {
    char     sql[257], schema[33];
    PGresult *res;

    strcpy(schema, get_schema(conn, tabname));
    sprintf(sql, "truncate table %s.%s", schema, tabname);
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
}
