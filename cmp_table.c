#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;
extern fcd_t *fcd_open;
extern table_t *tab_open;

void cmp_table(PGconn *conn) {

    fcd_t *fcd;
    table_t *tab;
    short reclen;
    unsigned char opcode[2], status_isam[2], status_bd[2], record_isam[MAX_REC_LEN+1], record_bd[MAX_REC_LEN+1];
    FILE *f;
    char logname[33];
    int err=0, c;

    if (fcd_open == NULL) {
        return;
    }
    fcd = fcd_open;
    tab = tab_open;

    if (dbg > 0) {
        fprintf(stderr, "%ld cmp [%s]\n", time(NULL), tab->name);
    }

    sprintf(logname, "%s_cmp.log", tab->name);
    if ((f = fopen(logname, "w")) == NULL) {
        return;
    }

    fcd->open_mode = 128;

    reclen = getshort(fcd->rec_len);
    record_isam[reclen] = 0;
    record_bd[reclen] = 0;

    putshort(opcode, OP_OPEN_INPUT);
    EXTFH(opcode, fcd);
    if (dbg > 2) {
        fprintf(stderr, "%ld cmp open isam %c%c %d\n", time(NULL), fcd->status[0], fcd->status[1], fcd->status[1]);
    }

    putshort(opcode, OP_START_GT);

    memset(fcd->record, 0, reclen);
    EXTFH(opcode, fcd);
    if (dbg > 2) {
        fprintf(stderr, "%ld cmp start isam %c%c %d\n", time(NULL), fcd->status[0], fcd->status[1], fcd->status[1]);
    }
    memcpy(status_isam, fcd->status, 2);
    memcpy(record_isam, fcd->record, reclen);

    memset(fcd->record, 0, reclen);
    op_start(conn, fcd, ">=");
    if (dbg > 2) {
        fprintf(stderr, "%ld cmp start bd %c%c %d\n", time(NULL), fcd->status[0], fcd->status[1], fcd->status[1]);
    }
    memcpy(status_bd, fcd->status, 2);
    memcpy(record_bd, fcd->record, reclen);

    if (memcmp(status_isam, status_bd, 2)) {
        fprintf(f, "start isam=%c%c bd=%c%c\n", status_isam[0], status_isam[1], fcd->status[0], fcd->status[1]);
        fclose(f);
        return;
    }

    putshort(opcode, OP_READ_NEXT);
    c = 0;
    while (true) {

        if (!memcmp(status_isam, ST_OK, 2) && (c <= 0)) {
            memcpy(fcd->record, record_isam, reclen);
            EXTFH(opcode, fcd);
            if (dbg > 2) {
                fprintf(stderr, "%ld cmp next isam %c%c [%s]\n", time(NULL), fcd->status[0], fcd->status[1], fcd->record);
            }
            memcpy(status_isam, fcd->status, 2);
            memcpy(record_isam, fcd->record, reclen);
        }

        if (!memcmp(status_bd, ST_OK, 2) && (c >= 0)) {
            memcpy(fcd->record, record_bd, reclen);
            op_next_prev(conn, fcd, 'n');
            if (dbg > 2) {
                fprintf(stderr, "%ld cmp next bd %c%c [%s]\n", time(NULL), fcd->status[0], fcd->status[1], fcd->record);
            }
            memcpy(status_bd, fcd->status, 2);
            memcpy(record_bd, fcd->record, reclen);
        }

        if (memcmp(fcd->status, status_isam, 2)) {
            fprintf(f, "next isam=%c%c bd=%c%c err=%d\n", status_isam[0], status_isam[1], fcd->status[0], fcd->status[1], ++err);
            break;

        } else if (!memcmp(status_isam, ST_OK, 2) && (c = memcmp(record_isam, record_bd, reclen))) {
            fprintf(f, "registros diferentes err=%d\n", ++err);
            fprintf(f, "isam [%s]\n", record_isam);
            fprintf(f, "bd   [%s]\n", record_bd);
        }

        if (memcmp(status_isam, ST_OK, 2) && memcmp(status_bd, ST_OK, 2)) {
            break;
        }
    }

    if (err == 0) {
        fprintf(f, "OK\n");
    }
    fclose(f);

}
