#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;
extern fcd_t *fcd_open;
extern table_t *tab_open;

void  load_table(PGconn *conn) {
    funcao = _LOAD_TABLE;    

    fcd_t fcd;
    table_t *tab;
    short reclen;
    unsigned char opcode[2];

    if (fcd_open == NULL) {
        return;
    }
    //fcd = fcd_open;
    memcpy(&fcd, fcd_open, sizeof(fcd_t));
    tab = tab_open;

    if (dbg > 0) {
        fprintf(flog, "%ld load [%s]\n", time(NULL), tab->name);
    }

    truncate_table(conn, tab->name);

    fcd.open_mode = 128;

    reclen = getshort(fcd.rec_len);
    memset(fcd.record, 0, reclen);

    putshort(opcode, OP_OPEN_INPUT);
    EXTFH(opcode, &fcd);
    if (fcd.status[0] == '9') {
        errorisam("load", opcode, &fcd);    
    }        
    if (dbg > 2) {
        fprintf(flog, "%ld load open %c%c %d\n", time(NULL), fcd.status[0], fcd.status[1], fcd.status[1]);
    }

    putshort(opcode, OP_START_GT);
    EXTFH(opcode, &fcd);
    if (fcd.status[0] == '9') {
        errorisam("load", opcode, &fcd);    
    }        
    if (dbg > 2) {
        fprintf(flog, "%ld load start %c%c %d\n", time(NULL), fcd.status[0], fcd.status[1], fcd.status[1]);
    }

    putshort(opcode, OP_READ_NEXT);
    EXTFH(opcode, &fcd);
    if (fcd.status[0] == '9') {
        errorisam("load", opcode, &fcd);    
    }        
    if (dbg > 2) {
        fprintf(flog, "%ld load next %c%c %d\n", time(NULL), fcd.status[0], fcd.status[1], fcd.status[1]);
    }

    while (fcd.status[0] == '0') {

        op_write(conn, &fcd);
        if (dbg > 2) {
            fprintf(flog, "%ld load write %c%c %d\n", time(NULL), fcd.status[0], fcd.status[1], fcd.status[1]);
        }

        putshort(opcode, OP_READ_NEXT);
        EXTFH(opcode, &fcd);
        if (fcd.status[0] == '9') {
            errorisam("load", opcode, &fcd);    
        }        
        if (dbg > 2) {
            fprintf(flog, "%ld load next %c%c %d\n", time(NULL), fcd.status[0], fcd.status[1], fcd.status[1]);
        }

    }

    putshort(opcode, OP_CLOSE);
    EXTFH(opcode, &fcd);
    if (dbg > 2) {
        fprintf(flog, "%ld load close %c%c %d\n", time(NULL), fcd.status[0], fcd.status[1], fcd.status[1]);
    }

}
