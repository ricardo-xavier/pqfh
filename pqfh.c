#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

// referencias:
// https://www.postgresql.org/docs/9.1/libpq-example.html

//extern void EXTFH(unsigned char *opcode, fcd_t *fcd);

PGconn *conn=NULL;
int dbg=-1;

#define VERSAO "v1.1.0 18/05/2019"

void EXTFH(unsigned char *opcode, fcd_t *fcd) {

    char           *conninfo;
    unsigned short op;
    PGresult       *res;

    if (dbg == -1) {
        char *env = getenv("PQFH_DBG");
        fprintf(stderr, "pqfh %s\n", VERSAO);
        if (env == NULL) {
            dbg = 0;
        } else {
            dbg = atoi(env);
        }
    }

    if (conn == NULL) {
        conninfo = strdup("dbname=integral user=postgres host=127.0.0.1 port=5432");

        if (dbg > 0) {
            fprintf(stderr, "%ld connect [%s]\n", time(NULL), conninfo);
        }
        conn = PQconnectdb(conninfo);
        if (PQstatus(conn) != CONNECTION_OK) {
            fprintf(stderr, "Erro na conexao com o banco de dados: %s\n%s\n",
                PQerrorMessage(conn), conninfo);
            exit(-1);
        }

        res = PQexec(conn, "BEGIN");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro ao iniciar a transacao: %s\n",
                PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);
            exit(-1);
        }
        PQclear(res);
    }

    op = getshort(opcode);
    switch (op) {

        case OP_OPEN_INPUT:
        case OP_OPEN_OUTPUT:
        case OP_OPEN_IO:
            op_open(conn, fcd, op);
            break;

        case OP_CLOSE:
            op_close(conn, fcd);
            break;

        case OP_START_GT:
            op_start_gt(conn, fcd);
            break;

        case OP_READ_NEXT:
            op_read_next(conn, fcd);
            break;

        case OP_READ_RANDOM:
            op_read_random(conn, fcd);
            break;

        case OP_REWRITE:
            op_rewrite(conn, fcd);
            break;

        case OP_WRITE:
            op_write(conn, fcd);
            break;

        case OP_DELETE:
            op_delete(conn, fcd);
            break;

        default:
            fprintf(stderr, "Comando nao implementado: %04x\n", op);
            PQfinish(conn);
            exit(-1);

    }

}
