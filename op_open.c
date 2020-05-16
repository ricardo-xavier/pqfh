#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;
extern fcd_t *fcd_open;
table_t *tab_open;
int ncomps=0;

bool op_open(PGconn *conn, fcd_t *fcd, unsigned short opcode) {

    char           filename[257], aux[257], *p;
    table_t        *tab;
    unsigned short fnlen;
    unsigned int   fileid;
    int            k, a;

    fcd_open = fcd;
    fnlen = getshort(fcd->file_name_len);
    memcpy(filename, (char *) fcd->file_name, fnlen);
    filename[fnlen] = 0;
    if ((p = strchr(filename, ' ')) != NULL) *p = 0;
    if ((p = strrchr(filename, '/')) != NULL) {
        strcpy(aux, p+1);
        strcpy(filename, aux);
    }
    
    if (dbg > 0) {
        fprintf(flog, "%ld op_open [%s] %04x %d %c\n", time(NULL), filename, opcode, (int) fcd->open_mode, fcd->sign);
    }

    if (fcd->open_mode != 128) {
        memcpy(fcd->status, ST_ALREADY_OPENED, 2);
        if (dbg > 0) {
            fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return false;
    }

    // aloca e inicializa a tabela
    tab = (table_t *) malloc(sizeof(table_t));
    tab_open = tab;
    strcpy(tab->name, filename);
    strcpy(tab->dictname, "");
    strcpy(tab->schema, "");
    tab->columns = NULL;
    tab->keys = NULL;
    tab->key_next = -1;
    tab->key_prev = -1;
    for (k=0; k<MAX_KEYS; k++) {
        tab->read_prepared[k] = false;
    }
    tab->upd_prepared = false;
    tab->ins_prepared = false;
    tab->del_prepared = false;
    tab->restart = 0;
    for (k=0; k<MAX_KEYS; k++) {
        tab->prms_random[k] = NULL;
    }
    tab->prms_rewrite = NULL;
    tab->prms_delete = NULL;
    tab->clones = NULL;
    tab->cursor = false;
    tab->advisory_lock = 0;
    tab->timestamp = time(NULL);
    tab->first = false;
    tab->num_apis = 0;
    for (a=0; a<MAX_APIS; a++) {
        tab->api[a][0] = 0;
        tab->columns_api[a] = NULL;
    }
    tab->partial_key = ncomps;
    ncomps = 0;

    fileid = (int) tab;
    putint(fcd->file_id, fileid);

#ifndef ISAM
    if (strcmp(tab->name, "pqfh")) {
        if (table_info(conn, tab, fcd)) {
            fcd->open_mode = opcode - 0xfa00;
            if (opcode == OP_OPEN_OUTPUT) {
                truncate_table(conn, tab->name);
            }
            memcpy(fcd->status, ST_OK, 2);
        } else {
            memcpy(fcd->status, ST_FILE_NOT_FOUND, 2);
            if ((fcd->isam != 'S') && (opcode != OP_OPEN_INPUT) && strcmp(tab->name, tab->dictname)) {
                create_table(conn, tab, fcd, opcode);
                table_info(conn, tab, fcd);
            } 
        }
        if (fcd->isam == 'S') {
#ifndef API
            free_tab(tab);
#endif
            return false;
        }
        fcd->sign = 'P';
    } else {
#endif
        fcd->open_mode = opcode - 0xfa00;
        memcpy(fcd->status, ST_OK, 2);
#ifndef ISAM
    }
#endif

    if (dbg > 0) {
        fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
    }

    return !strcmp(tab->name, "pqfh");
}

void set_partial(int _ncomps) {
    ncomps = _ncomps;
}
