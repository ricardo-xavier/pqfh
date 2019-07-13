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
        fprintf(stderr, "COMANDO PQFH: %s\n", fcd->record);
    }

    if (!memcmp(fcd->record, "WEAK:", 5)) {
        p = (char *) fcd->record + 5;
        weak = list2_append(weak, p, strlen(p));
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

    if (!memcmp(fcd->record, "LOAD:", 5)) {
        load_table(conn);
    }

    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
}
