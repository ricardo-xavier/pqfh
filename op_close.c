#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
list2_t *weak;

bool op_close(PGconn *conn, fcd_t *fcd) {
    funcao = _OP_CLOSE;    
    unsigned int fileid;
    table_t      *tab;
    char tabname[4097];

            short fnlen = getshort(fcd->file_name_len);
            char filename[257];
            memcpy(filename, (char *) fcd->file_name, fnlen);
            filename[fnlen] = 0;

fprintf(stderr, "OP_CLOSE %d [%s]\n", fcd->open_mode, filename);
    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_ALREADY_CLOSED, 2);
        if (dbg > 0) {
            short fnlen = getshort(fcd->file_name_len);
            char filename[257];
            memcpy(filename, (char *) fcd->file_name, fnlen);
            filename[fnlen] = 0;
            if (log_table(filename)) {
                fprintf(flog, "%ld op_close [%s] %d\n", time(NULL), filename, (int) fcd->open_mode);
                fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
            }    
        }
        return false;
    }

    fileid = getint(fcd->file_id);

    tab = (table_t *) fileid;
    strcpy(tabname, tab->name);
    if (dbg > 0) {
        if (log_table(tabname)) fprintf(flog, "%ld op_close [%s] %d %c\n", time(NULL), tab->name, (int) fcd->open_mode, fcd->sign);
    }

    if (!strcmp(tab->name, "pqfh")) {
        free_tab(tab);
        fcd->open_mode = 128;
        memcpy(fcd->status, ST_OK, 2);
        return true;
    }

    if (is_weak(tab->name)) {
        list2_t *ptr;
        for (ptr=list2_first(weak); ptr!=NULL; ptr=ptr->next) {
            char *buf = (char *) ptr->buf;
            if (!strcmp(buf, tab->name)) {
                weak = list2_remove(ptr);
                break;
            }
        }
    }

    if (tab->advisory_lock > 0) {
        unlock(fcd);
    }

    if (fcd->sign == 'P') {
        deallocate(conn, tab);
    }
    close_cursor(conn, tab);

    if (fcd->sign == 'P') {
        free_tab(tab);
    }
    fcd->open_mode = 128;
    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        if (log_table(tabname)) fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
    }

    return false;
}
