#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;
extern fcd_t *fcd_open;
extern table_t *tab_open;

void  load_table(PGconn *conn) {

    fcd_t *fcd;
    table_t *tab;
    short reclen;
    unsigned char opcode[2];

    if (fcd_open == NULL) {
        return;
    }
    fcd = fcd_open;
    tab = tab_open;

    if (dbg > 0) {
        fprintf(stderr, "load [%s] %ld\n", tab->name, time(NULL));
    }

    truncate_table(conn, tab->name);

    reclen = getshort(fcd->rec_len);
    memset(fcd->record, 0, reclen);

    putshort(opcode, OP_START_GT);
    EXTFH(opcode, fcd);
    if (dbg > 2) {
        fprintf(stderr, "load start %c%c %d\n", fcd->status[0], fcd->status[1], fcd->status[1]);
    }

    putshort(opcode, OP_READ_NEXT);
    EXTFH(opcode, fcd);
    if (dbg > 2) {
        fprintf(stderr, "load next %c%c %d\n", fcd->status[0], fcd->status[1], fcd->status[1]);
    }

    while (fcd->status[0] == '0') {

        op_write(conn, fcd);
        if (dbg > 2) {
            fprintf(stderr, "load write %c%c %d\n", fcd->status[0], fcd->status[1], fcd->status[1]);
        }

        putshort(opcode, OP_READ_NEXT);
        EXTFH(opcode, fcd);
        if (dbg > 2) {
            fprintf(stderr, "load next %c%c %d\n", fcd->status[0], fcd->status[1], fcd->status[1]);
        }

    }

}