#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;
extern int pending_commits;

void  copy_table(PGconn *conn, char *_source, char *_dest) {
    funcao = _COPY_TABLE;    
    char     sql[257], schema[33], *source, *dest;
    PGresult *res;

    source = strrchr(_source, '/');
    if (source == NULL) {
        source = _source;
    } else source++;
    dest = strrchr(_dest, '/');
    if (dest == NULL) {
        dest = _dest;
    } else dest++;

    strcpy(schema, get_schema(conn, dest));
    if (schema[0]) {
        // a tabela destino ja existe no banco
        sprintf(sql, "drop table %s.%s", schema, dest);
        if (dbg > 1) {
            fprintf(flog, "%ld %s\n", time(NULL), sql);
        }
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
            PQclear(res);
            return;
        }
        PQclear(res);
    }

    strcpy(schema, get_schema(conn, source));
    if (!schema[0]) {
        // a tabela origem nao existe
        return;
    }

    sprintf(sql, "create table %s.%s as select * from %s.%s", schema, dest, schema, source);
    if (dbg > 1) {
        fprintf(flog, "%ld %s\n", time(NULL), sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        errorbd(sql, res);    
        fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        PQclear(res);
        return;
    }
    PQclear(res);
    pending_commits++;
    commit();
}

void  truncate_table(PGconn *conn, char *_tabname) {
    funcao = _TRUNCATE_TABLE;    
    char     sql[257], schema[33], *tabname;
    PGresult *res;

    tabname = strrchr(_tabname, '/');
    if (tabname == NULL) {
        tabname = _tabname;
    } else tabname++;

    strcpy(schema, get_schema(conn, tabname));
    sprintf(sql, "truncate table %s.%s", schema, tabname);
    if (dbg > 1) {
        fprintf(flog, "%ld %s\n", time(NULL), sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        errorbd(sql, res);    
        fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
        PQclear(res);
        return;
    }
    PQclear(res);
}
