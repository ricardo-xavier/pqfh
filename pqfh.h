#ifndef PQFH_H
#define PQFH_H

#include "list2.h"
#include <libpq-fe.h>
#include <time.h>

FILE *flog;

#define bool unsigned char
#define false 0
#define true 1

#define MAX_COL_LEN  256
#define MAX_KEY_LEN  256
#define MAX_NAME_LEN 30
#define MAX_REC_LEN  19624
#define MAX_COLS     512
#define MAX_KEYS     16
#define MAX_COMPS    32
#define MAX_APIS     4

typedef struct {
    char    name[MAX_NAME_LEN+1];
    bool    convertida;
    char    schema[MAX_NAME_LEN+1];
    char    dictname[MAX_NAME_LEN+1];
    list2_t *columns;
    list2_t *keys;
    bool    read_prepared[MAX_KEYS];
    bool    upd_prepared;
    bool    ins_prepared;
    bool    del_prepared;
    char    restart;
    short   key_next;
    char    buf_next[MAX_KEY_LEN+1];
    char    rec_next[MAX_REC_LEN+1];
    short   key_prev;
    char    buf_prev[MAX_KEY_LEN+1];
    char    rec_prev[MAX_REC_LEN+1];
    list2_t *prms_random[MAX_KEYS];
    list2_t *prms_rewrite;
    list2_t *prms_delete;
    list2_t *clones;
    bool    cursor;
    int     advisory_lock;
    time_t  timestamp;

    const char *values[MAX_COLS];
    int        formats[MAX_COLS];
    int        lengths[MAX_COLS];
    char       bufs[MAX_COLS][MAX_COL_LEN+1];

    int oid;
    bool first;
    int num_apis;
    char api[MAX_APIS][MAX_NAME_LEN+1];
    char api_root[MAX_APIS][MAX_NAME_LEN+1];
    list2_t *columns_api[MAX_APIS];

    int partial_key;

} table_t;

typedef struct {
    char    schema[MAX_NAME_LEN+1];
    char    name[MAX_NAME_LEN+1];
    char    col1[MAX_NAME_LEN+1];
    char    col2[MAX_NAME_LEN+1];
    bool    key;
    bool    concat;
} clone_t;

typedef struct {
    char name[MAX_NAME_LEN+1];
    char tp;
    int len;
    int dec;
    int offset;
    bool pk;
    int p;
} column_t;

typedef struct {
    int id;
    int len;
    int ncomps;
    int ncols;
    column_t *columns[MAX_COMPS];
} _key_t;

#define MSGSZ 4096

#define CALLAPI_ENDERECO 1
#define CALLAPI_BEARER   2
#define CALLAPI_CALL     3

typedef struct msgbuf {
    long mtype;
    char mtext[MSGSZ];
} msg_t;

typedef struct {
    char api[33];
    char cmd;
    char json[MSGSZ-34];
} api_args_t;

typedef struct {
    unsigned char status[2];        /* 00 */
    unsigned char filler_1[3];      /* 02 */
    unsigned char organization;     /* 05 */
    unsigned char access_mode;      /* 06 */
    unsigned char open_mode;        /* 07 */
    unsigned char filler_2[3];      /* 08 */
    unsigned char file_name_len[2]; /* 11 */
    unsigned char filler_3[11];     /* 13 */
    unsigned char lock_mode;        /* 24 */
    unsigned char filler_4[8];      /* 25 */
    unsigned char fs_type;          /* 33 */
    unsigned char file_format;      /* 34 */
    unsigned char filler_5[3];      /* 35 */
    unsigned char rec_len[2];       /* 38 */
    unsigned char filler_6[7];      /* 40 */
    unsigned char reccording_mode;  /* 47 */
    unsigned char curr_rec_len[2];  /* 48 */
    unsigned char min_rec_len[2];   /* 50 */
    unsigned char key_id[2];        /* 52 */
    unsigned char filler_7[2];      /* 54 */
    unsigned char *record;          /* 56 */
    unsigned char *file_name;       /* 60 */
    unsigned char *kdb;             /* 64 */
    unsigned char filler_8[11];     /* 68 */
    unsigned char file_id[4];       /* 79 */
    unsigned char mode;             /* 83 */
    unsigned char filler_9[9];      /* 84 */
    unsigned char ignore_lock;      /* 93 */
    unsigned char filler_10[4];     /* 94 */
    unsigned char sign;             /* 98 */
    unsigned char isam;             /* 99 */
} fcd_t;

#define OP_OPEN_INPUT    0xfa00
#define OP_OPEN_OUTPUT   0xfa01
#define OP_OPEN_IO       0xfa02
#define OP_OPEN_EXTEND   0xfa03
#define OP_WRITE         0xfaf3
#define OP_REWRITE       0xfaf4
#define OP_CLOSE         0xfa80
#define OP_START_EQ      0xfae9
#define OP_START_GT      0xfaea
#define OP_START_GE      0xfaeb
#define OP_START_LT      0xfafe
#define OP_START_LE      0xfaff
#define OP_READ_NEXT     0xfaf5
#define OP_READ_PREVIOUS 0xfaf9
#define OP_READ_RANDOM   0xfaf6
#define OP_READ_LOCK     0xfada
#define OP_DELETE        0xfaf7
#define OP_UNLOCK        0xfa0e

#define ST_OK                "00"
#define ST_FILE_NOT_FOUND    "35"
#define ST_EOF               "10"
#define ST_REC_NOT_FOUND     "23"
#define ST_DUPL_KEY          "22"
#define ST_ERROR             "99"
#define ST_ALREADY_OPENED    "41"
#define ST_ALREADY_CLOSED    "42"
#define ST_NOT_OPENED_READ   "47"
#define ST_NOT_OPENED_WRITE  "48"
#define ST_NOT_OPENED_UPDEL  "49"
#define ST_LOCKED            "99"

#define DBG_UPD ((dbg_upd > 0) && (op == OP_WRITE || op == OP_REWRITE || op == OP_DELETE))

bool table_info(PGconn *conn, table_t *table, fcd_t *fcd);
char *get_schema(PGconn *conn, char *table);

unsigned short getshort(unsigned char *s);
unsigned int getint(unsigned char *s);
void putshort(unsigned char *s, unsigned short n);
void putint(unsigned char *s, unsigned int n);

void pq2cob(table_t *tab, PGresult *res, unsigned char *record, unsigned short reclen);

char *getkbuf(fcd_t *fcd, unsigned short keyid,  table_t *tab, unsigned short *keylen);
void getkeys(fcd_t *fcd, table_t *tab);
void getwhere(unsigned char *record, table_t *table, int keyid, char *op, char *where, char *order);
void getwhere_prepared(table_t *table, int keyid, char *where, int ini, char cmd);
void free_tab(table_t *tab);

list2_t *get_clones(char *tabela);
void replica_write(table_t *tab);
void replica_rewrite(table_t *tab);
void replica_delete(table_t *tab);
void replica_commit();
void replica_rollback();

void commit();
bool is_weak(char *table);
void command(PGconn *conn, fcd_t *fcd);
void pqfh_begin_transaction();
void pqfh_commit();
void pqfh_rollback();
void unlock(fcd_t *fcd);
void get_debug();
void errorisam(char *msg, unsigned char *opcode, fcd_t *fcd);
void errorbd(char *command, PGresult *res);

void deadlock_log(char *msg);
void deallocate(PGconn *conn, table_t *tab);
void close_cursor(PGconn *conn, table_t *tab);
void dbg_record(fcd_t *fcd);

bool op_open(PGconn *conn, fcd_t *fcd, unsigned short opcode);
bool op_close(PGconn *conn, fcd_t *fcd);
void op_start(PGconn *conn, fcd_t *fcd, char *op);
void op_next_prev(PGconn *conn, fcd_t *fcd, char dir);
void op_read_random(PGconn *conn, fcd_t *fcd, bool with_lock);
bool op_rewrite(PGconn *conn, fcd_t *fcd);
bool op_write(PGconn *conn, fcd_t *fcd);
void op_delete(PGconn *conn, fcd_t *fcd);
void create_table(PGconn *conn, table_t *tab, fcd_t *fcd, unsigned short opcode);
void copy_table(PGconn *conn, char *source, char *dest);
void load_table(PGconn *conn);
void cmp_table(PGconn *conn, bool sync);
void cmp_isam(PGconn *conn, char *filename);
void truncate_table(PGconn *conn, char *tabname);
void set_partial(int ncomps);

void thread_api_start(char cmd, table_t *tab, fcd_t *fcd);
void pqfh_call_java(char *endereco, char *operacao, char *metodo, char *json, char *bearer);

extern void EXTFH(unsigned char *opcode, fcd_t *fcd);

bool get_cache(table_t *table);
void put_cache(table_t *table);

void memfh_cbl(unsigned short op, fcd_t *fcd, char *filename);

#endif
