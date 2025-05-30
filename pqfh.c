#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#define MAIN
#include "pqfh.h"

// referencias:
// https://www.postgresql.org/docs/9.1/libpq-example.html
//
// create table tabela_api(tabela char(30) not null primary key, api char(30));
// insert into tabela_api values('sp05a51', 'planoGerencial');
//

#define VERSAO "v4.1.0 04/05/2025"

int dbg=-1;
int dbg_upd=-1;
int dbg_times=-1;
int dbg_cmp=-1;
int dbg_lock=-1;
char *dbg_tables=NULL;
char mode='I';
bool force_bd;
char *api=NULL;
int seqcmd=0;
fcd_t *fcd_open;
bool lock_manual=true;
bool executed=false;
char table_mode=0;
int funcao=-1;
time_t t0;

PGconn *conn=NULL;
#ifndef ISAM
PGconn *conn2=NULL;

int pending_commits = 0;
pthread_t thread_id;
pthread_mutex_t lock;

list2_t *weak=NULL;
extern bool replica_in_transaction;
extern bool force_partial;
extern bool fatal;

bool in_transaction=false;

list2_t *stlog=NULL;

list2_t *call_stack=NULL;

void program(char *name) {
    call_stack = list2_append(call_stack, name, strlen(name)+1);        
}

void exit_program() {
    if (call_stack == NULL) {
        return;
    }
    call_stack = list2_remove(call_stack);
}

void commit() {
    funcao = _COMMIT;    
    PGresult *res;
    if (dbg > 0 || (dbg_upd > 0) || (dbg_times > 0) || ((dbg_cmp > 0) && (pending_commits > 0))) {
        fprintf(flog, "%ld commit %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
    }
    res = PQexec(conn, "COMMIT");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        errorbd("COMMIT", res);    
    }
    PQclear(res);
    res = PQexec(conn, "BEGIN");
    PQclear(res);
    pending_commits = 0;
}

void *thread_commit(void *vargp) {
    funcao = _THREAD_COMMIT;    
    while (true) {
        sleep(1);
        if (dbg > 1) {
            fprintf(flog, "%ld pending_commits %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
        }
        if (fatal) {
            commit();
            exit(-1);
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
    funcao = _PQFH_BEGIN_TRANSACTION;    
    in_transaction = true;
    if (fatal) {
        exit(-1);
    }
    pthread_mutex_lock(&lock);
    commit();
    pthread_mutex_unlock(&lock);
}

void pqfh_commit() {
    funcao = _PQFH_COMMIT;    
    in_transaction = false;
    if (fatal) {
        commit();
        exit(-1);
    }
    pthread_mutex_lock(&lock);
    commit();
    pthread_mutex_unlock(&lock);
}

void pqfh_rollback() {
    funcao = _PQFH_ROLLBACK;    
    PGresult *res;
    in_transaction = false;
    if (fatal) {
        if (dbg > 0 || (dbg_upd > 0)) {
            fprintf(flog, "%ld rollback %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
        }
        res = PQexec(conn, "ROLLBACK");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            errorbd("ROLLBACK", res);    
        }
        PQclear(res);
        exit(-1);
    }
    pthread_mutex_lock(&lock);
    if (dbg > 0 || (dbg_upd > 0)) {
        fprintf(flog, "%ld rollback %d %s\n", time(NULL), pending_commits, in_transaction ? "TRANSACAO" : "AUTO_COMMIT");
    }
    res = PQexec(conn, "ROLLBACK");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        errorbd("ROLLBACK", res);    
    }
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

bool log_fcd(fcd_t *fcd) {
    char           filename[4097];
    unsigned short fnlen;
    if (dbg_tables == NULL) return true;    
    fnlen = getshort(fcd->file_name_len);
    if (fnlen == 0) return true;
    memcpy(filename, (char *) fcd->file_name, fnlen);
    filename[fnlen] = 0;
    return log_table(filename);        
}

bool log_table(char *filename) {
    char *p, aux[4097], *tabname=filename;
    if (dbg_tables == NULL || filename == NULL || filename[0] == 0 || filename[0] == ' ') return true;    
    if ((p = strrchr(filename, '/')) != NULL) {
        tabname = p+1;    
    }        
    sprintf(aux, ",%s,", tabname);
    return strstr(dbg_tables, aux) != NULL;        
}

void unlock(fcd_t *fcd) {
    funcao = _UNLOCK;    
    table_t *tab;
    char sql[4097];
    PGresult *res;
    unsigned int fileid = getint(fcd->file_id);
    tab = (table_t *) fileid;
    if (dbg > 0) {
        if (log_table(tab->name)) fprintf(flog, "%ld unlock [%s] %d %d\n", time(NULL), tab->name, tab->oid, tab->advisory_lock);
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

bool is_stlog(unsigned char st[2]) {
    list2_t *ptr;    
    unsigned char *aux;
    if (st[0] == '9') {
        return true;
    }
    for (ptr=stlog; ptr!=NULL; ptr=ptr->next) {
        aux = (unsigned char *) ptr->buf;    
        if ((aux[0] == st[0]) && (aux[1] == st[1])) {
            return true;
        }
    }
    return false;    
}

char get_mode() {
    int  fd, n, i;
    char mode='I', ch;
    bool lf=false;
    char st[3];

    if (access(".pqfh", F_OK) == -1) {
        return 'I';
    }
    fd = open(".pqfh", O_RDONLY);
    read(fd, &mode, 1);

    i = 0;
    st[2] = 0;
    while ((n = read(fd, &ch, 1)) > 0) {
        if (ch == '\n') {
            if (!lf) {
                lf = true;
                continue;
            } else {
                if (i == 2) {    
                    //printf("st=[%s]\n", st);
                    stlog = list2_append(stlog, st, 3);
                }
                break;
            }    
        }
        if (!lf) {
            continue;
        }
        if (ch == ' ') {
            if (i == 2) {    
                //printf("st=[%s]\n", st);
                stlog = list2_append(stlog, st, 3);
            }
            i = 0;
            continue;
        }
        if (i < 2) {
            st[i++] = ch;
        }
    }
    close(fd);
    stlog = list2_first(stlog);

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
    if (mode == 'L') {
        mode = 'W';
        if (dbg == 0) {
            putenv("PQFH_DBG=3");
            get_debug();
        }
    }
    return mode;
}

void get_debug() {
    t0 = time(NULL);    
    char *logname = getenv("PQFH_LOGNAME");
    char *env = getenv("PQFH_DBG");
    if (env == NULL) {
        dbg = 0;
    } else {
        dbg = atoi(env);
    }
    env = getenv("PQFH_DBG_TABLES");
    if (env == NULL) {
        dbg_tables = NULL;
    } else {
        dbg_tables = malloc(strlen(env) + 3);
        sprintf(dbg_tables, ",%s,", env);
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
}

FILE *erropen() {
    time_t t;
    struct tm *tm;
    int today, tomorow;
    char filename[4097];

    umask(0);
    t = time(NULL);
    tm = localtime(&t);
    today = tm->tm_wday;
    tomorow = today + 1;
    if (tomorow > 6) {
        tomorow = 0;
    }        

    sprintf(filename, "pqfh_%d.err", tomorow);
    if (access(filename, F_OK) != -1) {
        unlink(filename);
    }        

    sprintf(filename, "pqfh_%d.err", today);
    return fopen(filename, "a");

}

FILE *erropenbd() {
    time_t t;
    struct tm *tm;
    int today, tomorow;
    char filename[4097];

    umask(0);
    t = time(NULL);
    tm = localtime(&t);
    today = tm->tm_mday;
    tomorow = today + 1;

    sprintf(filename, "pqfh_bd_%d.err", tomorow);
    if (access(filename, F_OK) != -1) {
        unlink(filename);
    }        

    if (tomorow > 28) {
        tomorow = 1;
        sprintf(filename, "pqfh_bd_%d.err", tomorow);
        if (access(filename, F_OK) != -1) {
            unlink(filename);
        }        
    }        

    sprintf(filename, "pqfh_bd_%d.err", today);
    return fopen(filename, "a");

}

void errorisam(char *msg, unsigned char *opcode, fcd_t *fcd) {
    FILE *f;
    char user[4097], *u, filename[4097], progname[4097];
    short fnlen;
    unsigned short op;

    if (mode != 'W') {
        return;
    }    
    
    if ((f = erropen()) == NULL) {
        return;
    }
    if (call_stack == NULL) {
        strcpy(progname, "");
    } else {
        strcpy(progname, call_stack->buf);
    }    
    u = getenv("USER");
    if (u == NULL) {
        strcpy(user, "");
    } else {
        strcpy(user, u);
    }    
    fnlen = getshort(fcd->file_name_len);
    memcpy(filename, fcd->file_name, fnlen);
    filename[fnlen] = 0;
    op = getshort(opcode);
    fprintf(f, "%ld [%s] [%s] [%s] %04x %c%c\n\n", time(NULL), user, progname, filename, op, fcd->status[0], fcd->status[1]);
    fclose(f);
}

#ifndef ISAM
void errorbd(char *command, PGresult *res) {
    FILE *f;
    char user[4097], *u, progname[4097];

    if (mode != 'W') {
        return;
    }    

    if ((f = erropen()) == NULL) {
        return;
    }
    if (call_stack == NULL) {
        strcpy(progname, "");
    } else {
        strcpy(progname, call_stack->buf);
    }    
    u = getenv("USER");
    if (u == NULL) {
        strcpy(user, "");
    } else {
        strcpy(user, u);
    }    
    fprintf(f, "%ld BD [%s] [%s]\n%s\n%s\n\n", time(NULL), user, progname, command, PQerrorMessage(conn));
    fclose(f);

    if ((f = erropenbd()) == NULL) {
        return;
    }
    fprintf(f, "%ld [%s] [%s]\n%s\n%s\n\n", time(NULL), user, progname, command, PQerrorMessage(conn));
    fclose(f);
}

void warningbd(char *command, char *tabname, char *key, unsigned char status[2]) {
    FILE *f;
    char user[4097], *u, progname[4097];

    if (mode != 'W') {
        return;
    }    

    if ((f = erropen()) == NULL) {
        return;
    }
    if (call_stack == NULL) {
        strcpy(progname, "");
    } else {
        strcpy(progname, call_stack->buf);
    }    
    u = getenv("USER");
    if (u == NULL) {
        strcpy(user, "");
    } else {
        strcpy(user, u);
    }    
    fprintf(f, "%ld BD [%s] [%s] %s [%s] [%s] st=%c%c\n\n", time(NULL), user, progname, command, tabname, key, status[0], status[1]);
    fclose(f);

    if ((f = erropenbd()) == NULL) {
        return;
    }
    fprintf(f, "%ld [%s] [%s] %s [%s] [%s] st=%c%c\n\n", time(NULL), user, progname, command, tabname, key, status[0], status[1]);
    fclose(f);
}
#endif

void dbg_status(char *msg, fcd_t *fcd) {
    short reclen = getshort(fcd->rec_len);
    char aux[MAX_REC_LEN+1];
    memcpy(aux, fcd->record, reclen);
    aux[reclen] = 0;
    if (log_fcd(fcd)) fprintf(flog, "%s st=%c%c [%s] exec=%d\n\n", msg, fcd->status[0], fcd->status[1], aux, executed);
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

unsigned short op;
fcd_t *_fcd;

void trataTerm(int s) {
    fprintf(stderr, "SIGTERM\n");
}

void trataHUP(int s) {

    FILE *log114;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *usuario, progname[4097];

    fprintf(stderr, "SIGHUP\n");
    umask(0);
    log114 = fopen("pqfh114.log", "a");

    fprintf(log114, "%02d/%02d/%04d %02d:%02d:%02d\n", tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);

    if (call_stack == NULL) {
        strcpy(progname, "");
    } else {
        strcpy(progname, call_stack->buf);
    }    
    usuario = getenv("USER");
    if (usuario != NULL) {
        fprintf(log114, "usuario=%s programa=%s HANGUP\n\n", usuario, progname);
    } else {
        fprintf(log114, "HANGUP\n\n");
    }        

    fclose(log114);
    exit(-1);
}

void trata114(int s) {

    FILE *log114;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    short reclen, fnlen;
    char filename[4097], record[MAX_REC_LEN+1];
    char *usuario, progname[4097];

    umask(0);
    log114 = fopen("pqfh114.log", "a");
    setbuf(log114, NULL);
    fprintf(log114, "%02d/%02d/%04d %02d:%02d:%02d\n", tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);

    if (call_stack == NULL) {
        strcpy(progname, "");
    } else {
        strcpy(progname, call_stack->buf);
    }    
    usuario = getenv("USER");
    if (usuario != NULL) {
        fprintf(log114, "usuario=%s programa=%s\n", usuario, progname);
    }    

    fprintf(log114, "op=%04x\n", op);

    fnlen = getshort(_fcd->file_name_len);
    memcpy(filename, _fcd->file_name, fnlen);
    filename[fnlen] = 0;
    fprintf(log114, "arquivo=%s\n", filename);

    reclen = getshort(_fcd->rec_len);
    memcpy(record, _fcd->record, reclen);
    record[reclen] = 0;
    fprintf(log114, "registro=[%s]\n", filename);

    fprintf(log114, "funcao=%d\n", funcao);
    fprintf(log114, "tempo=%ld\n\n", t-t0);

    fclose(log114);
    exit(-1);
}    

void pqfh(unsigned char *opcode, fcd_t *fcd) {

    struct timeval tv1, tv2;
    short          reclen, fnlen;
    char           filename[4097], record[MAX_REC_LEN+1];
    long           tempo;
    unsigned char  open_mode, salva_isam;
    char           st[2];

#ifndef ISAM
    char           *conninfo;
    PGresult       *res;
    bool           ret;
    char           undo[MAX_REC_LEN+1];
#endif

    funcao = _PQFH;

    _fcd = fcd;
    gettimeofday(&tv1, NULL);
    if (dbg == -1) {
        get_debug();
        signal(SIGSEGV, trata114);
        signal(SIGHUP, trataHUP);
        signal(SIGTERM, trataTerm);
    }
    mode = get_mode();
    executed = false;

    // pega o nome do arquivo para gravar no log
    fnlen = getshort(fcd->file_name_len);
    memcpy(filename, fcd->file_name, fnlen);
    filename[fnlen] = 0;
    if (fnlen > 256) {
        putshort(fcd->file_name_len, 256);
        fnlen = getshort(fcd->file_name_len);
        memcpy(filename, fcd->file_name, fnlen);
        filename[fnlen] = 0;
    }    
    op = getshort(opcode);
    salva_isam = 0;

    if (fnlen > 5 && !memcmp(filename, "JSON:", 5)) {
        op_json(filename+5, op, fcd);
        return;
    }

    if ((op >= OP_OPEN_INPUT) && (op <= OP_OPEN_EXTEND)) {
        if (fcd->open_mode == 128) {
            fcd->mode = table_mode;
            fcd->isam = 0;
        }
    }    
    table_mode = 0;
    if (fcd->mode && (strchr("ABWIMF", fcd->mode) != NULL)) {
        mode = fcd->mode;
    }        

#ifndef ISAM
    if ((mode == 'M') || (mode == 'F'))  {
        memfh_cbl(mode, op, fcd, filename);
        return;
    }
#endif

    if ((mode == 'W') && (op == OP_OPEN_IO)) {
        salva_isam = fcd->isam;    
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
    if ((mode != 'I') && (op == OP_COMMIT)) {
        pqfh_commit();
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
            if (log_table(filename)) fprintf(flog, "%ld EXTFH %04x [%s] %d\n", time(NULL), op, filename, fcd->open_mode);
            dbg_record(fcd);
        }
        EXTFH(opcode, fcd);
        if (is_stlog(fcd->status)) {
            errorisam("pqfh", opcode, fcd);    
        }        
        if (dbg > 0 || DBG_UPD) {
            if (log_fcd(fcd)) fprintf(flog, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
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
            if (log_table(filename)) fprintf(flog, "%ld EXTFH %04x [%s] lock: status=%c%c\n", time(NULL), op, filename, fcd->status[0], fcd->status[1]);        
        }        
        if (op == OP_CLOSE) {
            mostra_tempos();
        }
#ifndef ISAM
        if (op == OP_UNLOCK) {
            pqfh_commit();
        }
#endif
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
            if (salva_isam != 0) {
                fcd->isam = salva_isam;
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
                if (log_table(filename)) fprintf(flog, "%ld EXTFH %04x [%s]\n", time(NULL), OP_READ_RANDOM, filename);
            }
            EXTFH(opcode, fcd);
            if (is_stlog(fcd->status)) {
                errorisam("pqfh", opcode, fcd);    
            }        
            if (dbg > 0) {
                if (log_fcd(fcd)) fprintf(flog, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
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
            if (log_table(filename)) fprintf(flog, "%ld EXTFH %04x [%s]\n", time(NULL), op, filename);
            dbg_record(fcd);
        }
        EXTFH(opcode, fcd);
        if (is_stlog(fcd->status)) {
            errorisam("pqfh", opcode, fcd);    
        }        
        if (dbg > 0 || DBG_UPD) {
            if (log_fcd(fcd)) fprintf(flog, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
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

    if (fatal) {
        exit(-1);
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
                if (is_stlog(fcd->status)) {
                    errorisam("pqfh", opcode, fcd);    
                }        
                if (dbg > 0) {
                    if (log_fcd(fcd)) fprintf(flog, "%ld EXTFH status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
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
                if (is_stlog(fcd->status)) {
                    errorisam("pqfh", opcode, fcd);    
                }        
                if (memcmp(fcd->status, ST_OK, 2)) {
                    // erro no isam
                    // fecha e retorna o erro do isam
                    if (dbg > 0) {
                        if (log_fcd(fcd)) fprintf(flog, "%ld desfaz open %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
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
            if (is_stlog(fcd->status)) {
                errorisam("pqfh", opcode, fcd);    
            }        
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
                    if (log_fcd(fcd)) fprintf(flog, "%ld desfaz rewrite %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                }

                // salva o registro atual e o status
                memcpy(record, fcd->record, reclen);
                memcpy(st, fcd->status, 2);

                // regrava o registro anterior
                memcpy(fcd->record, undo, reclen);
                EXTFH(opcode, fcd);
                if (is_stlog(fcd->status)) {
                    errorisam("pqfh", opcode, fcd);    
                }        

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
                    if (log_fcd(fcd)) fprintf(flog, "%ld desfaz write %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                }
                memcpy(st, fcd->status, 2);
                putshort(opcode, OP_DELETE);
                EXTFH(opcode, fcd);
                if (is_stlog(fcd->status)) {
                    errorisam("pqfh", opcode, fcd);    
                }        
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
        case OP_DELETE_FILE:
            op_delete_file(conn, fcd);
            break;

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
                    if (log_fcd(fcd)) fprintf(flog, "%ld desfaz delete %c%c\n", time(NULL), fcd->status[0], fcd->status[1]);
                }

                // salva o registro atual e o status
                memcpy(record, fcd->record, reclen);
                memcpy(st, fcd->status, 2);

                // insere o registro novamente no isam
                putshort(opcode, OP_WRITE);
                EXTFH(opcode, fcd);
                if (is_stlog(fcd->status)) {
                    errorisam("pqfh", opcode, fcd);    
                }        
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
            tempo_rewrite += tempo;
            qtde_rewrite++;
            if (mode == 'W') {
                break;
            }    
            break;

        case OP_WRITE:
            tempo_write += tempo;
            qtde_write++;
            break;
            if (mode == 'W') {
                break;
            }    

        case OP_DELETE:
            tempo_delete += tempo;
            qtde_delete++;
            if (mode == 'W') {
                break;
            }    
            break;

    }

    if ((mode != 'W') && (dbg_cmp > 0)) {
        dbg_status("", fcd);
    }
    if ((dbg_lock > 0) && (fcd->status[0] == '9')) {
        if (log_table(filename)) fprintf(flog, "%ld %04x [%s] lock: status=%c%c\n", time(NULL), op, filename, fcd->status[0], fcd->status[1]);        
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

/*
void pqfh_split(char *filename) {
    char *p;
    p = strchr(filename, ' ');
    if (p != NULL) *p = 0;
    pqfh_split_file(filename);
}
*/

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
// 3.0.4  - 15/10 - compilado sem ignorelock
// 3.0.5  - 16/10 - forcar commit no unlock do isam
// 3.0.6  - 18/10 - limite do sql no load e nao fazer lock no random em modo W
// 3.1.1  - 21/10 - correcoes no load
// 3.2.0  - 22/10 - gravar o log por dia da semana
// 3.3.2  - 31/10 - ncomps aumentado para 32
// 3.3.3  - 31/10 - carregar os metadados depois do create table
// 3.4.0  - 03/11 - modo de execucao por tabela - remocao do reopen
// 3.4.1  - 04/11 - alteracao do byte da fcd utilizado para armazenar o modo de execucao da tabela
// 3.4.2  - 05/11 - correcao no teste do sign no close
// 3.4.3  - 06/11 - alterar o modo com mais de uma tabela no open
// 3.4.4  - 18/11 - prefixo < 4
// 3.5.0  - 21/11 - memfh
// 3.5.1  - 27/11 - setar file status no memfh
// 3.5.2  - 22/12 - correcoes no memfh
// 3.5.5  - 07/01 - nao trocar o mode e isam no open se o arquivo nao estiver fechado
//                  open i-o       seta isam = S
//                  open input     estava resetando o isam e retornando 41
//                  close          estava abortando no op_close porque nao estava tratando como isam
// 3.6.0  - 16/03 - delete file e mode L
// 3.7.0  - 18/04 - cache de metadados
// 3.7.1  - 22/04 - remover o cache no create table
// 3.7.2  - 15/05 - inicializar dictname e schema no open - estava abortando no cache put
// 3.7.3  - 27/05 - inicializar columns e keys no cache_put
// 3.7.4  - 28/05 - nao gravar tabelas nao convertidas no cache
// 3.8.0  - 29/05 - tratamento do 114
// 3.8.1  - 30/05 - desalocar tabela pqfh no close
// 3.9.0  - 28/06 - hangup
// 3.9.1  - 15/08 - chave com campo decimal
// 3.10.0 - 03/09 - validacao de campos numericos
// 3.10.1 - 05/09 - nao corrigir o conteudo na validacao
// 3.10.2 - 08/09 - corrigir se nao for chave
// 3.10.3 - 08/09 - abortar se for erro na chave
// 3.10.5 - 09/09 - nao fazer lock no commit depois de erro fatal
// 3.10.6 - 22/09 - programa abortando depois de status 41 com W
// 3.11.0 - 23/09 - selecao de status para log
// 3.11.1 - 03/10 - contabilizar tempos de atualizacoes no modo W
// 3.11.2 - 05/10 - a tabela estava ficando aberta no load
// 3.12.0 - 27/10 - log separado para erros de banco
// 3.12.1 - 05/11 - delete com decimal na chave e warnings de banco
// 3.12.2 - 06/11 - aumento da variavel sql no delete e nome da tabela no warning
// 3.12.5 - 18/11 - aumento do tamanho do nome das colunas
// 3.12.6 - 24/11 - mostrar o nome da tabela no warningbd
// 3.13.0 - 14/03 - commit no sigterm
// 3.14.0 - 23/05 - call_stack
// 3.16.0 - 03/09 - json
// 4.0.0  - 05/01 - changelog
// 4.1.0  - 04/05 - apply changelog
