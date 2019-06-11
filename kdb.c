#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "list2.h"

extern int dbg;

void getkeys(fcd_t *fcd, table_t *tab) {

    unsigned short nkeys;
    unsigned short cdaoffset, ncomps, offset, len, k, c;
    unsigned char  *kda, *cda;
    list2_t *ptr;
    column_t *col;
    _key_t key;

    tab->keys = NULL;

    // key definition block
    nkeys = getshort(fcd->kdb + 6);

    for (k=0; k<nkeys; k++) {

        // key definition area
        kda = fcd->kdb + 14 + (k * 16);
        ncomps = getshort(kda + 0);
        cdaoffset = getshort(kda + 2);

        key.id = k;
        key.ncomps = ncomps;
        key.ncols = 0;
        key.len = 0;

        if (ncomps == 1) {
            // chave simples ou composta sem split

            // component definition area 
            cda = fcd->kdb + cdaoffset;
            offset = getint(cda + 2);
            len = getint(cda + 6);
            key.len += len;

            for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {
                col = (column_t *) ptr->buf;
                if (k == 0) {
                    col->pk = true;
                }
                if (col->offset == offset) {
                    while (len > 0) {
                        col = (column_t *) ptr->buf;
                        if (k == 0) {
                            col->pk = true;
                        }
                        key.columns[key.ncols++] = col;
                        len -= col->len;
                        ptr = ptr->next;
                    }
                    break;
                }
            }

        } else {

            // split
            for (c=0; c<ncomps; c++) {

                // component definition area 
                cda = fcd->kdb + cdaoffset + (c * 10);
                offset = getint(cda + 2);
                len = getint(cda + 6);
                key.len += len;

                for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {
                    col = (column_t *) ptr->buf;
                    if (col->offset == offset) {
                        if (k == 0) {
                            col->pk = true;
                        }
                        key.columns[key.ncols++] = col;
                        len -= col->len;
                        break;
                    }
                }
            }
        }
        tab->keys = list2_append(tab->keys, (_key_t *)&key, sizeof(_key_t));
    }
    tab->keys = list2_first(tab->keys);

    if (dbg > 2) {
        for (ptr=tab->keys; ptr!=NULL; ptr=ptr->next) {
            _key_t *key = (_key_t *) ptr->buf;
            fprintf(stderr, "key %d %d %d\n", key->id, key->ncomps, key->ncols);
            for (c=0; c<key->ncols; c++) {
                fprintf(stderr, "    %s\n", key->columns[c]->name);
            }
        }
    }
}

void adiciona_comp(unsigned char *record, _key_t key, int c, char *op, char *where, char *order) {

    char aux[257];
    unsigned char  buf[257];
    column_t *col;

    col = key.columns[c];
    memcpy(buf, record+col->offset, col->len);
    buf[col->len] = 0;

    if (c == (key.ncols - 1)) {
        if (col->tp == 'n') {
            sprintf(aux, "%s %s %s", col->name, op, buf);
        } else {
            sprintf(aux, "%s %s '%s'", col->name, op, buf);
        }
        strcat(where, aux);
        strcat(order, col->name);
        if (op[0] == '<') {
            strcat(order, " desc");
        }
        return;
    }

    if ((op[0] != '>') && (op[0] != '<')) {
        if (col->tp == 'n') {
            sprintf(aux, "%s = %s and ", col->name, buf);
        } else {
            sprintf(aux, "%s = '%s' and ", col->name, buf);
        }
        strcat(where, aux);
        strcat(order, col->name);
        if (op[0] == '<') {
            strcat(order, " desc");
        }
        strcat(order, ",");
        adiciona_comp(record, key, c+1, op, where, order);
        return;
    }

    if (c > 0) {
        strcat(where, " or ");
    }

    if (col->tp == 'n') {
        sprintf(aux, "%s %c %s or ( %s = %s and ", col->name, op[0], buf, col->name, buf);
    } else {
        sprintf(aux, "%s %c '%s' or ( %s = '%s' and ", col->name, op[0], buf, col->name, buf);
    }
    strcat(where, aux);
    strcat(order, col->name);
    if (op[0] == '<') {
        strcat(order, " desc");
    }
    strcat(order, ",");
    adiciona_comp(record, key, c+1, op, where, order);
    strcat(where, " ) ");
}

void getwhere(unsigned char *record, table_t *table, int keyid, char *op, char *where, char *order) {
    list2_t *ptr;
    _key_t *key;
    int k;

    strcpy(where, "");
    strcpy(order, "");
    for (ptr=table->keys, k=0; ptr!=NULL; ptr=ptr->next, k++) {
        if (k == keyid) {
            key = (_key_t *) ptr->buf;
            adiciona_comp(record, *key, 0, op, where, order);
            break;
        }
    }
}

int seq;

void adiciona_comp_prepared(table_t *tab, _key_t key, int c, char *where, char cmd) {

    char aux[257];
    column_t *col;

    col = key.columns[c];

    if (c == (key.ncols - 1)) {
        sprintf(aux, "%s = $%d", col->name, ++seq);
        strcat(where, aux);
        switch (cmd) {
            case 'u':
                tab->prms_rewrite = list2_append(tab->prms_rewrite, col, sizeof(column_t));
                break;
            case 'd':
                tab->prms_delete = list2_append(tab->prms_delete, col, sizeof(column_t));
                break;
            default:
                tab->prms = list2_append(tab->prms, col, sizeof(column_t));
                break;
        }
        return;
    }

    sprintf(aux, "%s = $%d and ", col->name, ++seq);
    strcat(where, aux);
    switch (cmd) {
        case 'u':
            tab->prms_rewrite = list2_append(tab->prms_rewrite, col, sizeof(column_t));
            break;
        case 'd':
            tab->prms_delete = list2_append(tab->prms_delete, col, sizeof(column_t));
            break;
        default:
            tab->prms = list2_append(tab->prms, col, sizeof(column_t));
            break;
    }
    adiciona_comp_prepared(tab, key, c+1, where, cmd);

}

void getwhere_prepared(table_t *tab, int keyid, char *where, int ini, char cmd) {
    list2_t *ptr;
    _key_t *key;
    int k;

    switch (cmd) {
        case 'u':
            tab->prms_rewrite = list2_free(tab->prms_rewrite);
            break;
        case 'd':
            tab->prms_delete = list2_free(tab->prms_delete);
            break;
        default:
            tab->prms = list2_free(tab->prms);
            break;
    }
    strcpy(where, "");
    seq = ini;
    for (ptr=tab->keys, k=0; ptr!=NULL; ptr=ptr->next, k++) {
        if (k == keyid) {
            key = (_key_t *) ptr->buf;
            adiciona_comp_prepared(tab, *key, 0, where, cmd);
            break;
        }
    }

    switch (cmd) {
        case 'u':
            tab->prms_rewrite = list2_first(tab->prms_rewrite);
            break;
        case 'd':
            tab->prms_delete = list2_first(tab->prms_delete);
            break;
        default:
            tab->prms = list2_first(tab->prms);
            break;
    }
}

static char kbuf[257];

char *getkbuf(fcd_t *fcd, unsigned short keyid,  table_t *tab, unsigned short *keylen) {
    int      k, c, offset;
    list2_t  *ptr;
    _key_t   *key;
    column_t *col;

    for (ptr=tab->keys, k=0; ptr!=NULL; ptr=ptr->next, k++) {
        if (k != keyid) {
            continue;
        }
        key = (_key_t *) ptr->buf;
        offset = 0;
        for (c=0; c<key->ncols; c++) {
            col = key->columns[c];
            memcpy(kbuf+offset, fcd->record+col->offset, col->len);
            offset += col->len;
        }
        kbuf[offset] = 0;
    }
    *keylen = offset;
    return kbuf;
}

