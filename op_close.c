#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;

void op_close(PGconn *conn, fcd_t *fcd) {
    unsigned int fileid;
    table_t      *tab;

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_close [%s] %ld\n", tab->name, time(NULL));
    }

    deallocate(conn, tab);
    close_cursor(conn, tab);

    // desaloca a tabela
    tab->columns = list2_free(tab->columns);
    tab->keys = list2_free(tab->keys);
    tab->prms = list2_free(tab->prms);
    tab->prms_rewrite = list2_free(tab->prms_rewrite);
    tab->prms_delete = list2_free(tab->prms_delete);
    tab->clones = list2_free(tab->clones);

    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
}
