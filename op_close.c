#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
list2_t *weak;

bool op_close(PGconn *conn, fcd_t *fcd) {
    unsigned int fileid;
    table_t      *tab;
    int          k;

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_close [%s] %d %ld\n", tab->name, (int) fcd->open_mode, time(NULL));
    }

    if (!strcmp(tab->name, "pqfh")) {
        return true;
    }

    if (is_weak(tab->name)) {
        list2_t *ptr;
        for (ptr=list2_first(weak); ptr!=NULL; ptr=ptr->next) {
            char *buf = (char *) ptr->buf;
            if (!strcmp(buf, tab->name)) {
                weak = list2_remove(weak, ptr);
                break;
            }
        }
    }

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_ALREADY_CLOSED, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return false;
    }

    deallocate(conn, tab);
    close_cursor(conn, tab);

    // desaloca a tabela
    tab->columns = list2_free(tab->columns);
    tab->keys = list2_free(tab->keys);
    for (k=0; k<MAX_KEYS; k++) {
        tab->prms_random[k] = list2_free(tab->prms_random[k]);
    }
    tab->prms_rewrite = list2_free(tab->prms_rewrite);
    tab->prms_delete = list2_free(tab->prms_delete);
    tab->clones = list2_free(tab->clones);

    fcd->open_mode = 128;
    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
    free(tab);
    return false;
}
