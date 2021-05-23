#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pqfh.h"

extern int dbg;
extern bool lock_manual;
extern char mode;
extern char table_mode;
extern bool usecache;
#ifndef ISAM
extern list2_t *weak;
#endif

void command(PGconn *conn, fcd_t *fcd) {
    funcao = _COMMAND;    
    char *p, aux[257];
#ifndef ISAM
    unsigned short reclen;
    char *q;
#endif

    if ((p = strchr((char *) fcd->record, ' ')) != NULL) *p = 0;

    if (dbg > 0) {
        fprintf(flog, "%ld COMANDO PQFH: %s\n", time(NULL), fcd->record);
    }

#ifndef ISAM
    if (!memcmp(fcd->record, "WEAK:", 5)) {
        strcpy(aux, (char *) fcd->record+5);
        if ((p = strchr(aux, ' ')) != NULL) *p = 0;
        weak = list2_append(weak, aux, strlen(aux)+1);
    }

    if (!memcmp(fcd->record, "MODE:", 5)) {
        table_mode = fcd->record[5];    
    }

    if (!memcmp(fcd->record, "PROGRAM:", 8)) {
        program((char *) (fcd->record+8));
    }

    if (!memcmp(fcd->record, "EXIT_PROGRAM", 12)) {
        exit_program();
    }

    if (!memcmp(fcd->record, "FREE", 4)) {
        table_mode = 'F';
    }

    if (!memcmp(fcd->record, "COPY:", 5)) {
        char salva_mode = mode;
        mode = 'B';   
        reclen = getshort(fcd->rec_len);
        memcpy(aux, fcd->record, reclen);
        aux[reclen] = 0;
        p = aux + 5;
        q = strchr(p, ':');
        *q++ = 0;
        copy_table(conn, p, q);
        commit();
        mode = salva_mode;
    }

    if (!memcmp(fcd->record, "LOAD", 4)) {
        char salva_mode = mode;
        mode = 'B';   
        load_table(conn);
        commit();
        mode = salva_mode;
    }

    if (!memcmp(fcd->record, "CACHE_START", 11)) {
        usecache = true;    
    }        

    if (!memcmp(fcd->record, "CACHE_STOP", 10)) {
        usecache = false;    
    }        
#endif

    if (!memcmp(fcd->record, "CMPISAM", 7)) {
        strcpy(aux, (char *) fcd->record+8);
        if ((p = strchr(aux, ' ')) != NULL) *p = 0;
        cmp_isam(conn, aux);
#ifndef ISAM
    } else if (!memcmp(fcd->record, "CMP", 3)) {
        char salva_mode = mode;
        mode = 'B';   
        cmp_table(conn, false);
        mode = salva_mode;

    } else if (!memcmp(fcd->record, "SYNC", 4)) {
        char salva_mode = mode;
        mode = 'B';   
        cmp_table(conn, true);
        commit();
        mode = salva_mode;
#endif
    }

#ifndef ISAM
    if (!memcmp(fcd->record, "BEGIN TRANSACTION", 17)) {
        pqfh_begin_transaction();
    }

    if (!memcmp(fcd->record, "COMMIT", 6)) {
        commit();
    }

    if (!memcmp(fcd->record, "ROLLBACK", 8)) {
        pqfh_rollback();
    }

    if (!memcmp(fcd->record, "PARTIAL:", 8)) {
        strcpy(aux, (char *) fcd->record+8);
        if ((p = strchr(aux, ' ')) != NULL) *p = 0;
        set_partial(atoi(aux));
    }

    if (!memcmp(fcd->record, "LOCK_MANUAL", 11)) {
        lock_manual = true;
    }

    if (!memcmp(fcd->record, "LOCK_AUTOMATIC", 14)) {
        lock_manual = false;
    }
#endif

    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
    }
}
