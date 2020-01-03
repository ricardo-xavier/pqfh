#include <string.h>
#include "pqfh.h"

extern int dbg;

void pq2cob(table_t *tab, PGresult *res, unsigned char *record, unsigned short reclen) {

    list2_t  *ptr;
    column_t *col;
    int      c=0, offset=0, len;
    char     aux[MAX_REC_LEN+1], *p;

    for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {
        col = (column_t *) ptr->buf;

        strcpy(aux, PQgetvalue(res, 0, c));
        len = strlen(aux);
        if (dbg > 2) {
            fprintf(flog, "    %d %d %s %c %d,%d [%s]\n", c, offset, col->name, col->tp, col->len, col->dec, aux);
        }

        switch (col->tp) {

            case 'n':
                p = aux;
                if (aux[0] == '-') {
                    p++;
                    len--;
                }
                if (col->dec == 0) {
                    memset(record + offset, '0', col->len - len);
                    memcpy(record + offset + (col->len - len), p, len);
                } else {
                    memset(record + offset, '0', col->len - len + 1);
                    memcpy(record + offset + (col->len - len + 1), p, len - col->dec - 1);
                    memcpy(record + offset + (col->len - len + 1) + (len - col->dec - 1), p + len - col->dec, col->dec);
                }
                if (p != aux) {
                    record[offset + col->len - 1] |= 0x40;
                }
                break;

            default:
                memcpy(record + offset, aux, len);
                memset(record + offset + len, ' ', col->len - len);
                break;

        }

        c++;
        offset += col->len;

    }
    //fprintf(flog, "[%s]\n", record);
}
