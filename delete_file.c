#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern int pending_commits;

void op_delete_file(PGconn *conn, fcd_t *fcd) {
    funcao = _OP_DELETE_FILE;    

    char     filename[257], aux[257], *p;
    table_t *tab;
    unsigned short fnlen;
    int ncomps=0, k, a;
    char     sql[257];
    PGresult *res;

    fnlen = getshort(fcd->file_name_len);
    memcpy(filename, (char *) fcd->file_name, fnlen);
    filename[fnlen] = 0;
    if ((p = strchr(filename, ' ')) != NULL) *p = 0;
    if ((p = strrchr(filename, '/')) != NULL) {
        strcpy(aux, p+1);
        strcpy(filename, aux);
    }
    
    if (dbg > 0) {
        fprintf(flog, "%ld op_delete_file [%s] %d\n", time(NULL), filename, (int) fcd->open_mode);
    }

    memcpy(fcd->status, ST_ERROR, 2);
    if (fcd->open_mode != 128) {
        if (dbg > 0) {
            fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
        }
        return;
    }

    // aloca e inicializa a tabela
    tab = (table_t *) malloc(sizeof(table_t));
    strcpy(tab->name, filename);
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

    if (table_info(conn, tab, fcd)) {
        sprintf(sql, "drop table %s.%s", tab->schema, tab->name);
        res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            errorbd(sql, res);    
            fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn), sql);
            PQclear(res);
            return;
        }
        PQclear(res);
        pending_commits++;
        commit();
    }        

    free_tab(tab);
    memcpy(fcd->status, ST_OK, 2);
}
