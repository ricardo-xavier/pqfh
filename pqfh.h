#ifndef PQFH_H
#define PQFH_H

#include "list2.h"
#include <libpq-fe.h>

#define bool unsigned char
#define false 0
#define true 1

#define MAX_KEY_LEN  256
#define MAX_NAME_LEN 30
#define MAX_REC_LEN  4096

typedef struct {
    char    name[MAX_NAME_LEN+1];
    char    dictname[MAX_NAME_LEN+1];
    list2_t *columns;
    list2_t *keys;
    short   key_read;
    bool    read_prepared;
    bool    upd_prepared;
    bool    ins_prepared;
    bool    del_prepared;
    short   key_next;
    char    buf_next[MAX_KEY_LEN+1];
    short   key_prev;
    char    buf_prev[MAX_KEY_LEN+1];
    char    *bufs[256];
    list2_t *prms;
    list2_t *prms_rewrite;
    list2_t *prms_delete;
} table_t;

typedef struct {
    char name[MAX_NAME_LEN+1];
    char tp;
    int len;
    int dec;
    int offset;
} column_t;

typedef struct {
    int id;
    int len;
    int ncomps;
    int ncols;
    column_t *columns[16];
} _key_t;

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
    unsigned char filler_9[10];     /* 83 */
    unsigned char ignore_lock;      /* 93 */
    unsigned char filler_10[5];     /* 94 */
    unsigned char isam;             /* 99 */
} fcd_t;

#define OP_OPEN_INPUT    0xfa00
#define OP_OPEN_OUTPUT   0xfa01
#define OP_OPEN_IO       0xfa02
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
#define OP_DELETE        0xfaf7

#define ST_OK             "00"
#define ST_FILE_NOT_FOUND "35"
#define ST_EOF            "10"
#define ST_REC_NOT_FOUND  "23"
#define ST_DUPL_KEY       "22"
#define ST_ERROR          "99"

bool table_info(PGconn *conn, table_t *table, fcd_t *fcd);
char *get_schema(PGconn *conn, char *table);
column_t *get_col_at(table_t *table, unsigned int offset);

unsigned short getshort(unsigned char *s);
unsigned int getint(unsigned char *s);
void putshort(unsigned char *s, unsigned short n);
void putint(unsigned char *s, unsigned int n);

void pq2cob(table_t *tab, PGresult *res, unsigned char *record, unsigned short reclen);

void kdb(fcd_t *fcd, unsigned int *offset, unsigned int *len);
void getkeys(fcd_t *fcd, table_t *tab);
void getwhere(unsigned char *record, table_t *table, int keyid, char *op, char *where, char *order);
void getwhere_prepared(table_t *table, int keyid, char *where, int ini, char cmd);

void commit();

void op_open(PGconn *conn, fcd_t *fcd, unsigned short opcode);
void op_close(PGconn *conn, fcd_t *fcd);
void op_start(PGconn *conn, fcd_t *fcd, char *op);
void op_next_prev(PGconn *conn, fcd_t *fcd, char dir);
void op_read_random(PGconn *conn, fcd_t *fcd);
bool op_rewrite(PGconn *conn, fcd_t *fcd);
void op_write(PGconn *conn, fcd_t *fcd);
void op_delete(PGconn *conn, fcd_t *fcd);
void create_table(PGconn *conn, table_t *tab, fcd_t *fcd, unsigned short opcode);

extern void EXTFH(unsigned char *opcode, fcd_t *fcd);

#endif
