#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "pqfh.h"

// referencias:
// https://www.postgresql.org/docs/9.1/libpq-example.html

PGconn *conn=NULL;
PGconn *conn2=NULL;
int dbg=-1;
int dbg_times=-1;

int pending_commits = 0;
pthread_t thread_id;
pthread_mutex_t lock;

char backup[MAX_REC_LEN+1];

list2_t *weak=NULL;
extern bool replica_in_transaction;

#define VERSAO "v1.13.4 26/07/2019"

bool in_transaction=false;

void commit() {
    PGresult *res;
    if (dbg > 0) {
        fprintf(stderr, "%ld commit %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
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
            fprintf(stderr, "%ld pending_commits %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
        }
        if ((pending_commits > 0) && !in_transaction) {
            pthread_mutex_lock(&lock);
            commit();
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL; 
} 

void pqfh_begin_transaction() {
    in_transaction = true;
    pthread_mutex_lock(&lock);
    commit();
    pthread_mutex_unlock(&lock);
}

void pqfh_commit() {
    in_transaction = false;
    pthread_mutex_lock(&lock);
    commit();
    pthread_mutex_unlock(&lock);
}

void pqfh_rollback() {
    PGresult *res;
    in_transaction = false;
    pthread_mutex_lock(&lock);
    if (dbg > 0) {
        fprintf(stderr, "%ld rollback %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
    }
    res = PQexec(conn, "ROLLBACK");
    PQclear(res);
    res = PQexec(conn, "BEGIN");
    PQclear(res);
    pending_commits = 0;
    pthread_mutex_unlock(&lock);
}

void unlock(fcd_t *fcd) {
    table_t *tab;
    char sql[257];
    PGresult *res;
    unsigned int fileid = getint(fcd->file_id);
    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "%ld unlock [%s] %d %d\n", time(NULL), tab->name, tab->oid, tab->advisory_lock);
    }
    sprintf(sql, "SELECT pg_advisory_unlock(%d, %d)", tab->oid, tab->advisory_lock);
    res = PQexec(conn, sql);
    PQclear(res);
    tab->advisory_lock = 0;
    pending_commits++;
    commit();
}

long tempo_total=0, tempo_open=0, tempo_close=0, tempo_start=0, tempo_next_prev=0, tempo_read=0, 
    tempo_write=0, tempo_rewrite=0, tempo_delete=0, tempo_isam=0;
int  qtde_total=0, qtde_open=0, qtde_close=0, qtde_start=0, qtde_next_prev=0, qtde_read=0, 
    qtde_write=0, qtde_rewrite=0, qtde_delete=0, qtde_isam=0;

char get_mode() {
    int  fd;
    char mode='I';
    if (access(".pqfh", F_OK) == -1) {
        return 'I';
    }
    fd = open(".pqfh", O_RDONLY);
    read(fd, &mode, 1); 
    close(fd);
    return mode;
}

void get_debug() {
    char *env = getenv("PQFH_DBG");
    if (env == NULL) {
        dbg = 0;
    } else {
        dbg = atoi(env);
    }
    if (dbg > 0) {
        fprintf(stderr, "%ld pqfh %s\n", time(NULL), VERSAO);
    }
    env = getenv("PQFH_DBG_TIMES");
    if (env == NULL) {
        dbg_times = 0;
    } else {
        dbg_times = atoi(env);
    }
}

void pqfh(unsigned char *opcode, fcd_t *fcd) {

    char           *conninfo;
    unsigned short op;
    unsigned char  open_mode;
    PGresult       *res;
    bool           ret;
    char           aux[MAX_REC_LEN+1];
    short          reclen;
    char           st[2], mode;
    struct timeval tv1, tv2;
    long tempo;

    gettimeofday(&tv1, NULL);
    if (dbg == -1) {
        get_debug();
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
            fprintf(stderr, "%ld Erro na conexao com o banco de dados: %s\n%s\n",
                time(NULL), PQerrorMessage(conn), conninfo);
            exit(-1);
        }

        res = PQexec(conn, "BEGIN");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "%ld Erro ao iniciar a transacao: %s\n",
                time(NULL), PQerrorMessage(conn));
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
                fprintf(stderr, "%ld Erro na conexao com o banco de dados: %s\n%s\n",
                    time(NULL), PQerrorMessage(conn2), conninfo);
                exit(-1);
            }

            res = PQexec(conn2, "BEGIN");
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                fprintf(stderr, "%ld Erro ao iniciar a transacao: %s\n",
                    time(NULL), PQerrorMessage(conn2));
                PQclear(res);
                PQfinish(conn2);
                exit(-1);
            }
            PQclear(res);
        }

        pthread_mutex_init(&lock, NULL);
        pthread_create(&thread_id, NULL, thread_commit, NULL);
    }

    op = getshort(opcode);
    mode = get_mode();

    if ((mode == 'I') || (fcd->isam == 'S')) {
        if (dbg > 0) {
            short fnlen = getshort(fcd->file_name_len);
            char aux[257];
            memcpy(aux, fcd->file_name, fnlen);
            aux[fnlen] = 0;
            fprintf(stderr, "%ld EXTFH %04x [%s]\n\n", time(NULL), op, aux);
        }
        EXTFH(opcode, fcd);
        if (dbg > 0) {
            fprintf(stderr, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        gettimeofday(&tv2, NULL);
        tempo = ((tv2.tv_sec * 1000000) + tv2.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        tempo_isam += tempo;
        tempo_total += tempo;
        qtde_isam++;
        qtde_total++;
        return;
    }

    pthread_mutex_lock(&lock);
    switch (op) {

        case OP_OPEN_INPUT:
        case OP_OPEN_OUTPUT:
        case OP_OPEN_IO:
            if (op_open(conn, fcd, op)) {
                break; // command
            }
            if (fcd->isam == 'S') {
                EXTFH(opcode, fcd);
                if (dbg > 0) {
                    fprintf(stderr, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
                }
                break;
            }
            if (mode == 'B') {
                break;
            }
            if (!memcmp(fcd->status, ST_OK, 2)) {
                // open com sucesso no banco
                // executa o open no isam
                fcd->open_mode = 128;
                EXTFH(opcode, fcd);
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    // fecha e retorna o erro do isam
                    if (dbg > 0) {
                        fprintf(stderr, "%ld desfaz open %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                    }
                    memcpy(st, fcd->status, 2);
                    op_close(conn, fcd);
                    memcpy(fcd->status, st, 2);
                }
            }
            break;

        case OP_CLOSE:
            open_mode = fcd->open_mode; 
            if (pending_commits > 0) {
                commit();
            }
            if (op_close(conn, fcd)) {
                break;
            }
            if (mode == 'B') {
                break;
            }
            fcd->open_mode = open_mode; 
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

        case OP_READ_LOCK:
            op_read_random(conn, fcd, true);
            break;

        case OP_UNLOCK:
            unlock(fcd);
            break;

        case OP_READ_RANDOM:
            op_read_random(conn, fcd, false);
            break;

        case OP_REWRITE:
            ret = op_rewrite(conn, fcd);
            if (mode == 'B') {
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
                        fprintf(stderr, "%ld desfaz rewrite %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
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
                    if (replica_in_transaction) {
                        replica_rollback();
                    }
                } else {
                    // sucesso no isam
                    if (dbg > 1) {
                        fprintf(stderr, "%ld rewrite confirmado\n", time(NULL));
                    }
                    if (replica_in_transaction) {
                        replica_commit();
                    }
                }
            }
            break;

        case OP_WRITE:
            if (op_write(conn, fcd)) {
                break; // command
            }
            if (mode == 'B') {
                break;
            }
            if (!memcmp(fcd->status, ST_OK, 2)) {
                // insert com sucesso no banco
                // executa o write no isam
                EXTFH(opcode, fcd);
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    if (dbg > 0) {
                        fprintf(stderr, "%ld desfaz write %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                    }
                    memcpy(st, fcd->status, 2);
                    op_delete(conn, fcd);
                    memcpy(fcd->status, st, 2);
                    if (replica_in_transaction) {
                        replica_rollback();
                    }
                } else {
                    // sucesso no isam
                    if (dbg > 1) {
                        fprintf(stderr, "%ld write confirmado\n", time(NULL));
                    }
                    if (replica_in_transaction) {
                        replica_commit();
                    }
                }
            }
            break;

        case OP_DELETE:
            op_delete(conn, fcd);
            if (mode == 'B') {
                break;
            }
            if (!memcmp(fcd->status, ST_OK, 2)) {
                // delete com sucesso no banco
                // executa o delete no isam
                EXTFH(opcode, fcd);
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    if (dbg > 0) {
                        fprintf(stderr, "%ld desfaz delete %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                    }
                    memcpy(st, fcd->status, 2);
                    op_write(conn, fcd);
                    memcpy(fcd->status, st, 2);
                    if (replica_in_transaction) {
                        replica_rollback();
                    }
                } else {
                    // sucesso no isam
                    if (dbg > 1) {
                        fprintf(stderr, "%ld delete confirmado\n", time(NULL));
                    }
                    if (replica_in_transaction) {
                        replica_commit();
                    }
                }
            }
            break;

        default:
            fprintf(stderr, "Comando nao implementado: %04x\n", op);
            //PQfinish(conn);
            //exit(-1);

    }

    gettimeofday(&tv2, NULL);
    tempo = ((tv2.tv_sec * 1000000) + tv2.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
    tempo_total += tempo;
    qtde_total++;
    switch (op) {

        case OP_OPEN_INPUT:
        case OP_OPEN_OUTPUT:
        case OP_OPEN_IO:
            tempo_open += tempo;
            qtde_open++;
            break;

        case OP_CLOSE:
            tempo_close += tempo;
            qtde_close++;
            if (dbg_times > 0) {
                fprintf(stderr, "tempo_total=%ld %d\n", tempo_total, qtde_total);
                fprintf(stderr, "tempo_open=%ld %d\n", tempo_open, qtde_open);
                fprintf(stderr, "tempo_close=%ld %d\n", tempo_close, qtde_close);
                fprintf(stderr, "tempo_start=%ld %d\n", tempo_start, qtde_start);
                fprintf(stderr, "tempo_next_prev=%ld %d\n", tempo_next_prev, qtde_next_prev);
                fprintf(stderr, "tempo_read=%ld %d\n", tempo_read, qtde_read);
                fprintf(stderr, "tempo_write=%ld %d\n", tempo_write, qtde_write);
                fprintf(stderr, "tempo_rewrite=%ld %d\n", tempo_rewrite, qtde_rewrite);
                fprintf(stderr, "tempo_delete=%ld %d\n", tempo_delete, qtde_delete);
                fprintf(stderr, "tempo_isam=%ld %d\n", tempo_isam, qtde_isam);
            }
            break;

        case OP_START_GT:
        case OP_START_GE:
        case OP_START_LT:
        case OP_START_LE:
        case OP_START_EQ:
            tempo_start += tempo;
            qtde_start++;
            break;

        case OP_READ_NEXT:
        case OP_READ_PREVIOUS:
            tempo_next_prev += tempo;
            qtde_next_prev++;
            break;

        case OP_READ_RANDOM:
            tempo_read += tempo;
            qtde_read++;
            break;

        case OP_REWRITE:
            tempo_rewrite += tempo;
            qtde_rewrite++;
            break;

        case OP_WRITE:
            tempo_write += tempo;
            qtde_write++;
            break;

        case OP_DELETE:
            tempo_delete += tempo;
            qtde_delete++;
            break;

    }

    pthread_mutex_unlock(&lock);

}

bool is_weak(char *table) {
    list2_t *ptr;
    char    *tab;

    for (ptr=list2_first(weak); ptr!=NULL; ptr=ptr->next) {
        tab = (char *) ptr->buf;
        if (!strcmp(table, tab)) {
            return true;
        }    
    }
    return false;
}

// 1.9.0  - 30/06 - weak
// 1.9.1  - 06/07 - verificar se a tabela esta aberta em todas as operacoes
// 1.9.2  - 06/07 - read random com mais de uma chave
// 1.10.0 - 07/07 - preparacao dos comandos de replicacao
// 1.10.1 - 08/07 - correcoes dos problemas passados pelo Breno por email
// 1.11.0 - 11/07 - estava testando o buf_next no start menor
//                  prepared statement por chave no random
//                  comandos copy e truncate
//                  modo de operacao
// 1.11.1 - 13/07 - commit no copy
// 1.12.0 - 13/07 - comando load
//                  lock de registro
// 1.12.1 - 16/07 - correcao no copy table com path
//                  nao executar create table se for isam
//                  fazer rollback no lock timeout
// 1.12.3 - 20/07 - aumento do buffer para 4k no pq2cob
//                  nao executar close na tabela pqfh
//                  correcao ao pegar o nome das tabelas no copy
// 1.12.4 - 22/07 - tratar chave comp ncomps > 1 e com o ultimo componente concatenado
// 1.12.5 - 23/07 - flag for_update para desalocar o registro
// 1.12.5 - 23/07 - flag for_update para desalocar o registro
// 1.13.0 - 24/07 - formatacao no libcobolpost
// 1.12.5 - 23/07 - flag for_update para desalocar o registro
// 1.13.0 - 24/07 - formatacao no libcobolpost
// 1.13.1 - 25/07 - advisory_lock
// 1.13.2 - 26/07 - fim de string no nome da tabela para o weak
// 1.13.4 - 26/07 - time no stderr
