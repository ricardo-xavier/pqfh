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
//
// create table tabela_api(tabela char(30) not null primary key, api char(30));
// insert into tabela_api values('sp05a51', 'planoGerencial');
//

#define VERSAO "v3.0.3 12/10/2019"

int dbg=-1;
int dbg_upd=-1;
int dbg_times=-1;
int dbg_cmp=-1;
int dbg_lock=-1;
char mode='I';
bool force_bd;
char *api=NULL;
bool reopen=false;
int seqcmd=0;
fcd_t *fcd_open;
bool lock_manual=true;
bool executed=false;

PGconn *conn=NULL;
#ifndef ISAM
PGconn *conn2=NULL;

int pending_commits = 0;
pthread_t thread_id;
pthread_mutex_t lock;

list2_t *weak=NULL;
extern bool replica_in_transaction;
extern bool force_partial;

bool in_transaction=false;

void commit() {
    PGresult *res;
    if (dbg > 0 || (dbg_upd > 0) || (dbg_times > 0) || ((dbg_cmp > 0) && (pending_commits > 0))) {
        fprintf(flog, "%ld commit %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
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
            fprintf(flog, "%ld pending_commits %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
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
    if (dbg > 0 || (dbg_upd > 0)) {
        fprintf(flog, "%ld rollback %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
    }
    res = PQexec(conn, "ROLLBACK");
    PQclear(res);
    res = PQexec(conn, "BEGIN");
    PQclear(res);
    pending_commits = 0;
    pthread_mutex_unlock(&lock);
}

void warning(void *arg, const char *message) {
    if (dbg > 0) {
        fprintf(flog, "%s\n", message);
    }
}

void unlock(fcd_t *fcd) {
    table_t *tab;
    char sql[257];
    PGresult *res;
    unsigned int fileid = getint(fcd->file_id);
    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(flog, "%ld unlock [%s] %d %d\n", time(NULL), tab->name, tab->oid, tab->advisory_lock);
    }
    sprintf(sql, "SELECT pg_advisory_unlock(%d, %d)", tab->oid, tab->advisory_lock);
    res = PQexec(conn, sql);
    PQclear(res);
    tab->advisory_lock = 0;
    pending_commits++;
/*
    if (dbg > 0 || (dbg_upd > 0) || (dbg_times > 0)) {
        fprintf(stderr, "commit unlock\n");
    }
    commit();
*/
}
#endif

long tempo_total=0, tempo_open=0, tempo_close=0, tempo_start=0, tempo_next_prev=0, tempo_read=0, 
    tempo_write=0, tempo_rewrite=0, tempo_delete=0, tempo_isam=0, tempo_cobolpost=0;
int  qtde_total=0, qtde_open=0, qtde_close=0, qtde_start=0, qtde_next_prev=0, qtde_read=0, 
    qtde_write=0, qtde_rewrite=0, qtde_delete=0, qtde_isam=0, qtde_cobolpost=0;

char get_mode() {
    int  fd;
    char mode='I';
    if (access(".pqfh", F_OK) == -1) {
        return 'I';
    }
    fd = open(".pqfh", O_RDONLY);
    read(fd, &mode, 1); 
    close(fd);
#ifndef ISAM
    if (mode == 'a') {
        mode = 'A';
        force_partial = true;
        lock_manual = true;
    }
    if (mode == 'b') {
        mode = 'B';
        force_partial = true;
        lock_manual = false;
    }
#endif
    return mode;
}

void get_debug() {
    char *logname = getenv("PQFH_LOGNAME");
    char *env = getenv("PQFH_DBG");
    if (env == NULL) {
        dbg = 0;
    } else {
        dbg = atoi(env);
    }
    env = getenv("PQFH_DBG_UPD");
    if (env == NULL) {
        dbg_upd = 0;
    } else {
        dbg_upd = atoi(env);
    }
    env = getenv("PQFH_DBG_TIMES");
    if (env == NULL) {
        dbg_times = 0;
    } else {
        dbg_times = atoi(env);
    }
    env = getenv("PQFH_DBG_CMP");
    if (env == NULL) {
        dbg_cmp = 0;
    } else {
        dbg_cmp = atoi(env);
    }
    env = getenv("PQFH_DBG_LOCK");
    if (env == NULL) {
        dbg_lock = 0;
    } else {
        dbg_lock = atoi(env);
    }
    flog = stderr;
    if ((dbg > 0) || (dbg_upd > 0) || (dbg_times > 0) || (dbg_cmp > 0) || (dbg_lock > 0))  {
        if (logname != NULL) {
            flog = fopen(logname, "w");
            if (flog == NULL) {
                flog = stderr;
            } else {
                setbuf(flog, NULL);
            }
        }
        fprintf(flog, "%ld pqfh %s\n", time(NULL), VERSAO);
    }
    env = getenv("PQFH_FORCE_BD");
    if (env == NULL) {
        force_bd = false;
    } else {
        force_bd = !strcmp(env, "S");
    }
    env = getenv("PQFH_API");
    if (env == NULL) {
        api = NULL;
    } else {
        api = env;
    }
    env = getenv("PQFH_REOPEN");
    if (env == NULL) {
        reopen = false;
    } else {
        reopen = env[0] == 'S';
    }
}

void dbg_status(char *msg, fcd_t *fcd) {
    short reclen = getshort(fcd->rec_len);
    char aux[MAX_REC_LEN+1];
    memcpy(aux, fcd->record, reclen);
    aux[reclen] = 0;
    fprintf(flog, "%s st=%c%c [%s] exec=%d\n\n", msg, fcd->status[0], fcd->status[1], aux, executed);
}

void mostra_tempos() {
    if (dbg_times == 0) {
        return;
    }
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
    fprintf(stderr, "tempo_cobolpost=%ld %d\n", tempo_cobolpost, qtde_cobolpost);
}


void pqfh(unsigned char *opcode, fcd_t *fcd) {

    struct timeval tv1, tv2;
    unsigned short op;
    short          reclen, fnlen;
    char           filename[257], record[MAX_REC_LEN+1];
    long           tempo;
    unsigned char  open_mode;
    char           st[2];

#ifndef ISAM
    char           *conninfo;
    PGresult       *res;
    bool           ret;
    char           undo[MAX_REC_LEN+1];
#endif

    gettimeofday(&tv1, NULL);
    if (dbg == -1) {
        get_debug();
    }
    mode = get_mode();
    executed = false;

    // pega o nome do arquivo para gravar no log
    fnlen = getshort(fcd->file_name_len);
    memcpy(filename, fcd->file_name, fnlen);
    filename[fnlen] = 0;
    op = getshort(opcode);

    if ((mode == 'W') && (op == OP_OPEN_IO)) {
        fcd->isam = 0;
    }        

    /*
    if (lock_manual) {
        fcd->lock_mode &= ~0x02;
        fcd->lock_mode |= 0x04;    
        if (op == OP_READ_LOCK) {
            fcd->ignore_lock |= 0x01;
        } else {
            fcd->ignore_lock &= ~0x01;
        }
    }
    */

#ifndef ISAM
    if ((conn == NULL) && (mode != 'I')) {
        // conecta ao banco de dados e configura a conexao
        conninfo = getenv("CONECTA_BD");
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

        PQsetNoticeProcessor(conn, warning, NULL);

        res = PQexec(conn, "set client_encoding to 'latin1'");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(flog, "%ld Erro ao configurar o encoding: %s\n",
                time(NULL), PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);
            exit(-1);
        }
        PQclear(res);

        res = PQexec(conn, "BEGIN");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(flog, "%ld Erro ao iniciar a transacao: %s\n",
                time(NULL), PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);
            exit(-1);
        }
        PQclear(res);

        // conecta ao banco de dados de replicacao
        conninfo = getenv("REPLICA_BD");
        if (conninfo != NULL) {
            if (dbg > 0) {
                fprintf(flog, "%ld connect replica [%s]\n", time(NULL), conninfo);
            }
            conn2 = PQconnectdb(conninfo);
            if (PQstatus(conn2) != CONNECTION_OK) {
                fprintf(flog, "%ld Erro na conexao com o banco de dados: %s\n%s\n",
                    time(NULL), PQerrorMessage(conn2), conninfo);
                exit(-1);
            }

            res = PQexec(conn2, "BEGIN");
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                fprintf(flog, "%ld Erro ao iniciar a transacao: %s\n",
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
#endif

    reclen = getshort(fcd->rec_len);

    if (dbg_cmp > 0) {
        memcpy(record, fcd->record, reclen);
        record[reclen] = 0;
        fprintf(flog, "%ld cmd=%d %04x [%s] [%s]\n", time(NULL), ++seqcmd, op, filename, record);
    }

    if (((mode == 'I') || (mode == 'W') || (fcd->isam == 'S')) && memcmp(filename, "pqfh", 4)) {
#ifdef API
        unsigned int fileid = getint(fcd->file_id);
        table_t *tab;
        if ((api != NULL) && (mode != 'I')) {
            tab = (table_t *) fileid;
        }
#endif
        if (dbg > 0 || DBG_UPD) {
            fprintf(flog, "%ld EXTFH %04x [%s]\n", time(NULL), op, filename);
            dbg_record(fcd);
        }
        EXTFH(opcode, fcd);
        if (dbg > 0 || DBG_UPD) {
            fprintf(flog, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        if (op <= OP_OPEN_EXTEND) {
            fcd_open = fcd;
        }
        gettimeofday(&tv2, NULL);
        tempo = ((tv2.tv_sec * 1000000) + tv2.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        tempo_isam += tempo;
        tempo_total += tempo;
        qtde_isam++;
        qtde_total++;
        if ((dbg_cmp > 0) && (mode != 'W')) {
            dbg_status("ISAM", fcd);
        }
        if ((dbg_lock > 0) && (fcd->status[0] == '9')) {
            fprintf(stderr, "%ld EXTFH %04x [%s] lock: status=%c%c\n", time(NULL), op, filename, fcd->status[0], fcd->status[1]);        
        }        
        if (op == OP_CLOSE) {
            mostra_tempos();
        }
#ifdef API
        if ((api != NULL) && (mode != 'I'))  {
            switch (op) {
                case OP_CLOSE:
                    free(tab);
                    fcd->isam = ' ';
                    break;
                case OP_WRITE:
                    if (tab->api[0] && !memcmp(fcd->status, ST_OK, 2)) {
                        thread_api_start('i', tab, fcd);
                    }
                    break;
            }
        }
#endif
        if (mode != 'W') {
            return;
        } else if (memcmp(fcd->status, ST_OK, 1)) {
            if (dbg_cmp > 0) {
                dbg_status("ISAM", fcd);
            }
            return;
        }    
    }

    if ((mode == 'W') && (fcd->isam == 'S') && strcmp(filename, "pqfh")) {
        if (dbg_cmp > 0) {
            dbg_status("ISAM", fcd);
        }
        return;
    }        

#ifndef ISAM
    if ((mode == 'A') && (fcd->isam != 'S') && strcmp(filename, "pqfh")
            && ((op == OP_WRITE) || (op == OP_REWRITE) || (op == OP_DELETE))) {

        if (op != OP_WRITE) {

            // salva o registro anterior para o caso de precisar desfazer
            memcpy(record, fcd->record, reclen);
            putshort(opcode, OP_READ_RANDOM);
            if (dbg > 0) {
                fprintf(flog, "%ld EXTFH %04x [%s]\n", time(NULL), OP_READ_RANDOM, filename);
            }
            EXTFH(opcode, fcd);
            if (dbg > 0) {
                fprintf(flog, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
            }
            putshort(opcode, op);

            if (memcmp(fcd->status, ST_OK, 2)) {
                gettimeofday(&tv2, NULL);
                tempo = ((tv2.tv_sec * 1000000) + tv2.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
                tempo_isam += tempo;
                tempo_total += tempo;
                qtde_isam++;
                qtde_total++;
                memcpy(fcd->record, record, reclen);
                return;
            }

            memcpy(undo, fcd->record, reclen);
            undo[reclen] = 0;
            memcpy(fcd->record, record, reclen);

        }

        // executa primeiro no ISAM
        if ((dbg > 0) || DBG_UPD) {
            fprintf(flog, "%ld EXTFH %04x [%s]\n", time(NULL), op, filename);
            dbg_record(fcd);
        }
        EXTFH(opcode, fcd);
        if (dbg > 0 || DBG_UPD) {
            fprintf(flog, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        gettimeofday(&tv2, NULL);
        tempo = ((tv2.tv_sec * 1000000) + tv2.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        tempo_isam += tempo;
        tempo_total += tempo;
        qtde_isam++;
        qtde_total++;

        if (memcmp(fcd->status, ST_OK, 2)) {
            // se der erro no ISAM nao executa no banco
            return;
        }
    }

    pthread_mutex_lock(&lock);
#endif
    memcpy(st, fcd->status, 2);
    open_mode = fcd->open_mode;
    switch (op) {
        case OP_OPEN_INPUT:
        case OP_OPEN_OUTPUT:
        case OP_OPEN_EXTEND:
            if (mode == 'W') {
                fcd->isam = 'S';    
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }        
        case OP_OPEN_IO:
            if (mode == 'W') {
                fcd->open_mode = 128;
            }    
            if (op_open(conn, fcd, op)) {
                if (mode == 'W') {
                    fcd->open_mode = open_mode;
                    memcpy(fcd->status, st, 2);
                    if (dbg_cmp > 0) {
                        dbg_status("ISAM", fcd);
                    }
                }    
                break; // command
            }
            if (mode == 'W') {
                if ((dbg_cmp > 0) && (fcd->isam != 'S')) {
                    dbg_status("BANCO", fcd);
                }
                fcd->open_mode = open_mode;
                memcpy(fcd->status, st, 2);
                if ((dbg_cmp > 0) && (fcd->isam == 'S')) {
                    dbg_status("BANCO", fcd);
                }
                break;
            }        
            if (fcd->isam == 'S') {
                EXTFH(opcode, fcd);
                if (dbg > 0) {
                    fprintf(flog, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
                }
                break;
            }
            if (mode == 'B') {
                break;
            }
#ifndef ISAM
            if (!memcmp(fcd->status, ST_OK, 2)) {
                // open com sucesso no banco
                // executa o open no isam
                fcd->open_mode = 128;
                EXTFH(opcode, fcd);
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    // fecha e retorna o erro do isam
                    if (dbg > 0) {
                        fprintf(flog, "%ld desfaz open %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                    }
                    memcpy(st, fcd->status, 2);
                    op_close(conn, fcd);
                    memcpy(fcd->status, st, 2);
                }
            }
#endif
            break;

#ifndef ISAM
        case OP_CLOSE:
            if (mode == 'W') {
                fcd->open_mode = 2;
            }    
/*
            if (pending_commits > 0) {
                commit();
            }
*/
            if (op_close(conn, fcd)) {
                if (mode == 'W') {    
                    fcd->open_mode = open_mode;
                    memcpy(fcd->status, st, 2);
                    if (dbg_cmp > 0) {
                        dbg_status("ISAM", fcd);
                    }
                }    
                break;
            }
            if (mode == 'W') {    
                if (dbg_cmp > 0) {
                    dbg_status("BANCO", fcd);
                }
                fcd->open_mode = open_mode;
                memcpy(fcd->status, st, 2);
                break; 
            }    
            if (mode == 'B') {
                break;
            }
            fcd->open_mode = open_mode; 
            EXTFH(opcode, fcd);
            break;

        case OP_START_GT:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_start(conn, fcd, ">");
            break;

        case OP_START_GE:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_start(conn, fcd, ">=");
            break;

        case OP_START_LT:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_start(conn, fcd, "<");
            break;

        case OP_START_LE:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_start(conn, fcd, "<=");
            break;

        case OP_START_EQ:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_start(conn, fcd, "=");
            break;

        case OP_READ_NEXT:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_next_prev(conn, fcd, 'n');
            break;

        case OP_READ_PREVIOUS:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_next_prev(conn, fcd, 'p');
            break;

        case OP_READ_LOCK:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_read_random(conn, fcd, true);
            break;

        case OP_UNLOCK:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            unlock(fcd);
            break;

        case OP_READ_RANDOM:
            if (mode == 'W') {
                if (dbg_cmp > 0) {
                    dbg_status("ISAM", fcd);
                }
                break;
            }    
            op_read_random(conn, fcd, false);
            break;

        case OP_REWRITE:
            ret = op_rewrite(conn, fcd);
            if (mode == 'W') {    
                if (dbg_cmp > 0) {
                    dbg_status("BANCO", fcd);
                }
                memcpy(fcd->status, st, 2);
                break;
            }    
            if ((mode != 'B') && (!ret || memcmp(fcd->status, ST_OK, 2))) {

                // erro no update do banco
                // desfaz o isam
                if (dbg > 1 || DBG_UPD) {
                    fprintf(flog, "%ld desfaz rewrite %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                }

                // salva o registro atual e o status
                memcpy(record, fcd->record, reclen);
                memcpy(st, fcd->status, 2);

                // regrava o registro anterior
                memcpy(fcd->record, undo, reclen);
                EXTFH(opcode, fcd);

                // restaura o registro atual e o status
                memcpy(fcd->record, record, reclen);
                memcpy(fcd->status, st, 2);

                break;
            }
            if (replica_in_transaction) {
                replica_commit();
            }
            break;
#endif

        case OP_WRITE:
            if (op_write(conn, fcd)) {
                if (mode == 'W') {    
                    if (dbg_cmp > 0) {
                        dbg_status("BANCO", fcd);
                    }
                    memcpy(fcd->status, st, 2);
                }    
                break; // command
            }
            if (mode == 'W') {    
                if (dbg_cmp > 0) {
                    dbg_status("BANCO", fcd);
                }
                memcpy(fcd->status, st, 2);
                break;
            }    
#ifndef ISAM
            if ((mode != 'B') && memcmp(fcd->status, ST_OK, 2)) {

                // erro no insert do banco
                // desfaz o isam
                if (dbg > 0 || DBG_UPD) {
                    fprintf(flog, "%ld desfaz write %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                }
                memcpy(st, fcd->status, 2);
                putshort(opcode, OP_DELETE);
                EXTFH(opcode, fcd);
                putshort(opcode, OP_WRITE);
                memcpy(fcd->status, st, 2);
                break;
            }
            if (replica_in_transaction) {
                replica_commit();
            }
#endif
            break;

#ifndef ISAM
        case OP_DELETE:
            op_delete(conn, fcd);
            if (mode == 'W') {    
                if (dbg_cmp > 0) {
                    dbg_status("BANCO", fcd);
                }
                memcpy(fcd->status, st, 2);
                break;
            }    
            if ((mode != 'B') && memcmp(fcd->status, ST_OK, 2)) {

                // erro no delete do banco
                // desfaz o isam
                if (dbg > 0 || DBG_UPD) {
                    fprintf(flog, "%ld desfaz delete %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                }

                // salva o registro atual e o status
                memcpy(record, fcd->record, reclen);
                memcpy(st, fcd->status, 2);

                // insere o registro novamente no isam
                putshort(opcode, OP_WRITE);
                EXTFH(opcode, fcd);
                putshort(opcode, OP_DELETE);

                // restaura o registro atual e o status
                memcpy(fcd->record, record, reclen);
                memcpy(fcd->status, st, 2);
                break;
            }
            if (replica_in_transaction) {
                replica_commit();
            }
            break;
#endif

        default:
            fprintf(flog, "Comando nao implementado: %04x\n", op);
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
        case OP_OPEN_EXTEND:
            tempo_open += tempo;
            qtde_open++;
            break;

        case OP_CLOSE:
            tempo_close += tempo;
            qtde_close++;
            mostra_tempos();
            break;

        case OP_START_GT:
        case OP_START_GE:
        case OP_START_LT:
        case OP_START_LE:
        case OP_START_EQ:
            if (mode == 'W') {
                break;
            }    
            tempo_start += tempo;
            qtde_start++;
            break;

        case OP_READ_NEXT:
        case OP_READ_PREVIOUS:
            if (mode == 'W') {
                break;
            }    
            tempo_next_prev += tempo;
            qtde_next_prev++;
            break;

        case OP_READ_RANDOM:
            if (mode == 'W') {
                break;
            }    
            tempo_read += tempo;
            qtde_read++;
            break;

        case OP_REWRITE:
            if (mode == 'W') {
                break;
            }    
            tempo_rewrite += tempo;
            qtde_rewrite++;
            break;

        case OP_WRITE:
            if (mode == 'W') {
                break;
            }    
            tempo_write += tempo;
            qtde_write++;
            break;

        case OP_DELETE:
            if (mode == 'W') {
                break;
            }    
            tempo_delete += tempo;
            qtde_delete++;
            break;

    }

    if ((mode != 'W') && (dbg_cmp > 0)) {
        dbg_status("", fcd);
    }
    if ((dbg_lock > 0) && (fcd->status[0] == '9')) {
        fprintf(stderr, "%ld %04x [%s] lock: status=%c%c\n", time(NULL), op, filename, fcd->status[0], fcd->status[1]);        
    }        
#ifndef ISAM
    pthread_mutex_unlock(&lock);
#endif
}

#ifndef ISAM
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
#endif

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
// 1.13.5 - 26/07 - ajuste no fechar do cobolpost para nao abortar se nao tiver conexao
// 1.14.0 - 26/07 - dbg_cmp
// 1.14.1 - 27/07 - retornar corretamente o status no start
// 1.15.0 - 27/07 - isam primeiro
// 1.15.3 - 30/07 - nao atualizar p_linha no cobolpost se nao for select
// 1.15.5 - 31/07 - nao considerar prefixo < 4
//                  testar se a origem e o destino existem no copy
// 1.15.6 - 01/08 - nao executar a atualizacao no ISAM se for o arquivo pqfh
// 1.16.0 - 04/08 - comando cmp
// 1.16.1 - 03/08 - correcao de 114 no fechar
// 1.16.2 - 06/08 - desalocar memoria em todos os pontos que usam o res
// 1.16.3 - 07/08 - mostrar warnings somente com dbg
// 1.17.0 - 08/08 - alteracao no comando do create table
// 2.0.0  - 11/08 - integracao com a API
// 2.0.1  - 14/08 - verificar o 128 no close antes de usar o nome da tabela
// 2.0.2  - 18/08 - verificar o 128 nas outras operacoes
// 2.0.3  - 19/08 - retornar antes do get_keys se as colunas estiverem nulas
// 2.0.4  - 26/08 - attach vm
// 2.1.0  - 27/08 - log de deadlock
// 2.2.0  - 28/08 - comparacao de ISAM
// 2.2.1  - 30/08 - resultado do CMP igual ao do CMPISAM
// 2.3.0  - 31/08 - correcoes nos comparadores e implementacao do SYNC
// 2.4.0  - 31/08 - dbg upd
// 2.5.0  - 03/09 - mais de uma api por tabela
// 2.5.1  - 08/09 - CMPISAM sem .pqfh
// 2.5.2  - 10/09 - registro estourando no debug
// 2.5.3  - 11/09 - gravar o log do cmp com fwrite
// 2.6.0  - 12/09 - PQFH_LOGNAME
// 2.6.1  - 13/09 - zerar o first antes do start
// 2.7.0  - 14/09 - reopen e nao fazer commit no close
// 2.7.2  - 16/09 - setar pending_commits no unlock, mas nao fazer commit
// 2.7.4  - 17/09 - zerar o restart no rewrite e no delete
// 2.7.5  - 18/09 - passar espacos no nome da tabela para o converteapi
// 2.7.6  - 19/09 - alem do restart, zerar tambem o key_next e key_prev
// 2.8.0  - 22/09 - pqfh isam
// 2.8.1  - 23/09 - inversao dos registros no cmp isam
// 2.9.0  - 26/09 - chave parcial
// 2.9.1  - 26/09 - comando PARTIAL
// 2.9.2  - 02/10 - problema com filename
// 2.9.4  - 02/10 - ignore lock no cmp_isam
// 2.10.0 - 06/10 - lock manual
// 3.0.0  - 07/10 - modo w
// 3.0.1  - 09/10 - cmp isam com nomes de arquivo de tamanhos diferentes
// 3.0.2  - 11/10 - retirar lock manual forcado
// 3.0.3  - 12/10 - sync e cmp com W
