#include "../pqfh.h"
#include <libpq-fe.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern table_t *tab_open;
extern fcd_t *fcd_open;
extern list2_t *sync_tables;
extern list2_t *sync_fcds;
extern FILE *flog;
extern int dbg;

void sync_add_table() {
    fcd_t    fcd;
    table_t  tab;

    if (tab_open == NULL) {
        return;
    }        
    if (dbg > 0) {
        short fnlen = getshort(fcd_open->file_name_len);
        char filename[256];
        memcpy(filename, fcd_open->file_name, fnlen);
        filename[fnlen] = 0;
        fprintf(flog, "sync add table %s.%s %s\n", tab_open->schema, tab_open->name, filename);
    }
    memcpy(&fcd, fcd_open, sizeof(fcd_t));
    sync_fcds = list2_append(sync_fcds, &fcd, sizeof(fcd_t));
    memcpy(&tab, tab_open, sizeof(table_t));
    sync_tables = list2_append(sync_tables, &tab, sizeof(table_t));
}
