#include <stdio.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern list2_t *weak;

void command(PGconn *conn, table_t *tab, fcd_t *fcd) {
    char *p, *q, aux[257];
    unsigned short reclen;

    if ((p = strchr((char *) fcd->record, ' ')) != NULL) *p = 0;

    if (dbg > 0) {
        fprintf(flog, "%ld COMANDO PQFH: %s\n", time(NULL), fcd->record);
    }

    if (!memcmp(fcd->record, "WEAK:", 5)) {
        strcpy(aux, (char *) fcd->record+5);
        if ((p = strchr(aux, ' ')) != NULL) *p = 0;
        weak = list2_append(weak, aux, strlen(aux)+1);
    }

    if (!memcmp(fcd->record, "COPY:", 5)) {
        reclen = getshort(fcd->rec_len);
        memcpy(aux, fcd->record, reclen);
        aux[reclen] = 0;
        p = aux + 5;
        q = strchr(p, ':');
        *q++ = 0;
        copy_table(conn, p, q);
    }

    if (!memcmp(fcd->record, "LOAD", 4)) {
        load_table(conn);
    }

    if (!memcmp(fcd->record, "CMPISAM", 7)) {
        strcpy(aux, (char *) fcd->record+8);
        if ((p = strchr(aux, ' ')) != NULL) *p = 0;
        cmp_isam(conn, aux);

    } else if (!memcmp(fcd->record, "CMP", 3)) {
        cmp_table(conn, false);

    } else if (!memcmp(fcd->record, "SYNC", 4)) {
        cmp_table(conn, true);
    }

    if (!memcmp(fcd->record, "BEGIN TRANSACTION", 17)) {
        pqfh_begin_transaction();
    }

    if (!memcmp(fcd->record, "COMMIT", 6)) {
        pqfh_commit();
    }

    if (!memcmp(fcd->record, "ROLLBACK", 8)) {
        pqfh_rollback();
    }

    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(flog, "%ld status=%c%c\n\n", time(NULL), fcd->status[0], fcd->status[1]);
    }
}
