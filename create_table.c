#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;

void create_table(PGconn *conn, table_t *tab, fcd_t *fcd, unsigned short opcode) {
    char     sql[257], schema[33];
    char     indexdef[257];
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
        PQclear(res);
        return;
    }
    PQclear(res);

    sprintf(sql, "declare cursor_indexes cursor for\n  select indexdef from pg_indexes where tablename='%s'", tab->dictname);
    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        return;
    }
    PQclear(res);

    for (;;) {
        res = PQexec(conn, "fetch next in cursor_indexes");
        if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
            break;
        }
        strcpy(indexdef, PQgetvalue(res, 0, 0));
        if (dbg > 1) {
            fprintf(stderr, "[%s]\n", indexdef);
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
        PQexec(conn, indexdef);
    }

    res = PQexec(conn, "CLOSE cursor_indexes");
    PQclear(res);

    res = PQexec(conn, "COMMIT");
    PQclear(res);

    memcpy(fcd->status, ST_OK, 2);
}
