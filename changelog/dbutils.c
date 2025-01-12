#include <stdio.h>
#include <libpq-fe.h>

extern int dbg;
extern PGconn *conn;
extern FILE *flog;

void notice(void *arg, const char *message) {
}

void dbconnect() {
    if (flog == NULL) {
        flog = stderr;
    }
    char *conninfo = getenv("CONECTA_BD");
    if (conninfo == NULL) {
        conninfo = strdup("dbname=integral user=postgres host=127.0.0.1 port=5432");
    }
    if (dbg > 0) {
        fprintf(flog, "%ld connect [%s]\n", time(NULL), conninfo);
    }
    conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(flog, "%ld Erro na conexao com o banco de dados: %s\n%s\n",
            time(NULL), PQerrorMessage(conn), conninfo);
        exit(-1);
    }
    PQsetNoticeProcessor(conn, notice, NULL);
    PGresult *res = PQexec(conn, "set client_encoding to 'latin1'");
    PQclear(res);
} 

PGresult *execcmd(char *cmd) {
    if (conn == NULL) {
        dbconnect();
    }    
    return PQexec(conn, cmd);
}    

void execcmd_or_exit(char *cmd) {
    if (conn == NULL) {
        dbconnect();
    }    
    PGresult *res = execcmd(cmd);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", 
            time(NULL), PQerrorMessage(conn), cmd);
        exit(-1);
    }    
    PQclear(res);
}    

static char t[4097];
char *rtrim(char *s, int n) {
    char *p;
    strncpy(t, s, n); 
    t[n] = 0;    
    if ((p = strchr(t, ' ')) != NULL) *p = 0;
    return t;
}
        
