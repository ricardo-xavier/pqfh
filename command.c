#include <stdio.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern list2_t *weak;

void command(PGconn *conn, table_t *tab, fcd_t *fcd) {
    char *p;

    if ((p = strchr((char *) fcd->record, ' ')) != NULL) *p = 0;

    printf("COMANDO PQFH: %s\n", fcd->record);

    if (!memcmp(fcd->record, "WEAK:", 5)) {
        p = (char *) fcd->record + 5;
        weak = list2_append(weak, p, strlen(p));
    }

    memcpy(fcd->status, ST_OK, 2);
    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
}
