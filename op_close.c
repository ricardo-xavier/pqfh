#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;

void op_close(PGconn *conn, fcd_t *fcd) {
    unsigned int fileid;
    table_t      *tab;

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    memcpy(fcd->status, ST_OK, 2);

    if (dbg > 0) {
        fprintf(stderr, "op_close [%s] %ld\n", tab->name, time(NULL));
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
}
