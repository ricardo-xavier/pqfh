#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "pqfh.h"

// referencias:
// https://www.postgresql.org/docs/9.1/libpq-example.html

PGconn *conn=NULL;
PGconn *conn2=NULL;
int dbg=-1;
bool isam=false;

int pending_commits = 0;
pthread_t thread_id;
pthread_mutex_t lock;

char backup[MAX_REC_LEN+1];

#define VERSAO "v1.6.3 14/06/2019"

void commit() {
    PGresult *res;
    if (dbg > 0) {
        fprintf(stderr, "commit %d\n", pending_commits);
    }
    res = PQexec(conn, "COMMIT");
    PQclear(res);
    res = PQexec(conn, "BEGIN");
    PQclear(res);
    pending_commits = 0;
}

void *thread_commit(void *vargp) {
    while (true) {
        sleep(1); 
        if (dbg > 1) {
            fprintf(stderr, "pending_commits %d\n", pending_commits);
        }
        if (pending_commits > 0) {
            pthread_mutex_lock(&lock);
            commit();
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL; 
} 

void pqfh(unsigned char *opcode, fcd_t *fcd) {

    char           *conninfo;
    unsigned short op;
    PGresult       *res;
    bool           ret;
    char           aux[MAX_REC_LEN+1];
    short          reclen;
    char           st[2];

    if (dbg == -1) {
        char *env = getenv("PQFH_DBG");
        fprintf(stderr, "pqfh %s\n", VERSAO);
        if (env == NULL) {
            dbg = 0;
        } else {
            dbg = atoi(env);
        }
        env = getenv("PQFH_ISAM");
        if (env == NULL) {
            isam = false;
        } else {
            isam = !strcasecmp(env, "S");
        }
    }

    if (conn == NULL) {
        conninfo = getenv("CONECTA_BD");
        if (conninfo == NULL) {
            conninfo = strdup("dbname=integral user=postgres host=127.0.0.1 port=5432");
        }

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

        conninfo = getenv("REPLICA_BD");
        if (conninfo != NULL) {
            if (dbg > 0) {
                fprintf(stderr, "%ld connect replica [%s]\n", time(NULL), conninfo);
            }
            conn2 = PQconnectdb(conninfo);
            if (PQstatus(conn2) != CONNECTION_OK) {
                fprintf(stderr, "Erro na conexao com o banco de dados: %s\n%s\n",
                    PQerrorMessage(conn2), conninfo);
                exit(-1);
            }

            res = PQexec(conn2, "BEGIN");
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                fprintf(stderr, "Erro ao iniciar a transacao: %s\n",
                    PQerrorMessage(conn2));
                PQclear(res);
                PQfinish(conn2);
                exit(-1);
            }
            PQclear(res);
        }

        pthread_mutex_init(&lock, NULL);
        pthread_create(&thread_id, NULL, thread_commit, NULL);
    }

    if (fcd->isam == 'S') {
        EXTFH(opcode, fcd);
        return;
    }

    pthread_mutex_lock(&lock);
    op = getshort(opcode);
    switch (op) {

        case OP_OPEN_INPUT:
        case OP_OPEN_OUTPUT:
        case OP_OPEN_IO:
            op_open(conn, fcd, op);
            if (fcd->isam == 'S') {
                EXTFH(opcode, fcd);
                break;
            }
            if (!isam) {
                break;
            }
            if (!memcmp(fcd->status, ST_OK, 2)) {
                // open com sucesso no banco
                // executa o open no isam
                EXTFH(opcode, fcd);
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    // fecha e retorna o erro do isam
                    if (dbg > 0) {
                        fprintf(stderr, "desfaz open %c%c\n", fcd->status[0], fcd->status[1]);
                    }
                    memcpy(st, fcd->status, 2);
                    op_close(conn, fcd);
                    memcpy(fcd->status, st, 2);
                }
            }
            break;

        case OP_CLOSE:
            op_close(conn, fcd);
            if (pending_commits > 0) {
                commit();
            }
            if (!isam) {
                break;
            }
            EXTFH(opcode, fcd);
            break;

        case OP_START_GT:
            op_start(conn, fcd, ">");
            break;

        case OP_START_GE:
            op_start(conn, fcd, ">=");
            break;

        case OP_START_LT:
            op_start(conn, fcd, "<");
            break;

        case OP_START_LE:
            op_start(conn, fcd, "<=");
            break;

        case OP_START_EQ:
            op_start(conn, fcd, "=");
            break;

        case OP_READ_NEXT:
            op_next_prev(conn, fcd, 'n');
            break;

        case OP_READ_PREVIOUS:
            op_next_prev(conn, fcd, 'p');
            break;

        case OP_READ_RANDOM:
            op_read_random(conn, fcd);
            break;

        case OP_REWRITE:
            ret = op_rewrite(conn, fcd);
            if (!isam) {
                break;
            }
            if (ret && !memcmp(fcd->status, ST_OK, 2)) {
                // update com sucesso no banco
                // executa o rewrite no isam
                // se o update nao foi executado devido a otimizacao entao ret = false e status = OK
                EXTFH(opcode, fcd);
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    if (dbg > 0) {
                        fprintf(stderr, "desfaz rewrite %c%c\n", fcd->status[0], fcd->status[1]);
                    }

                    // desfaz a alteracao
                    reclen = getshort(fcd->rec_len);

                    // salva o registro atual e o status
                    memcpy(aux, fcd->record, reclen);
                    memcpy(st, fcd->status, 2);

                    // regrava o registro anterior
                    memcpy(fcd->record, backup, reclen);
                    op_rewrite(conn, fcd);

                    // recupera o registro atual e o status
                    memcpy(fcd->record, aux, reclen);
                    memcpy(fcd->status, st, 2);
                } else {
                    // sucesso no isam
                    if (dbg > 1) {
                        fprintf(stderr, "rewrite confirmado\n");
                    }
                }
            }
            break;

        case OP_WRITE:
            op_write(conn, fcd);
            if (!isam) {
                break;
            }
            if (!memcmp(fcd->status, ST_OK, 2)) {
                // insert com sucesso no banco
                // executa o write no isam
                EXTFH(opcode, fcd);
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    if (dbg > 0) {
                        fprintf(stderr, "desfaz write %c%c\n", fcd->status[0], fcd->status[1]);
                    }
                    memcpy(st, fcd->status, 2);
                    op_delete(conn, fcd);
                    memcpy(fcd->status, st, 2);
                } else {
                    // sucesso no isam
                    if (dbg > 1) {
                        fprintf(stderr, "write confirmado\n");
                    }
                }
            }
            break;

        case OP_DELETE:
            op_delete(conn, fcd);
            if (!isam) {
                break;
            }
            if (!memcmp(fcd->status, ST_OK, 2)) {
                // delete com sucesso no banco
                // executa o delete no isam
                EXTFH(opcode, fcd);
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    if (dbg > 0) {
                        fprintf(stderr, "desfaz delete %c%c\n", fcd->status[0], fcd->status[1]);
                    }
                    memcpy(st, fcd->status, 2);
                    op_write(conn, fcd);
                    memcpy(fcd->status, st, 2);
                } else {
                    // sucesso no isam
                    if (dbg > 1) {
                        fprintf(stderr, "delete confirmado\n");
                    }
                }
            }
            break;

        default:
            fprintf(stderr, "Comando nao implementado: %04x\n", op);
            PQfinish(conn);
            exit(-1);

    }
    pthread_mutex_unlock(&lock);

}
