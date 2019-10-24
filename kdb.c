#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "list2.h"

extern int dbg;
extern bool partial_weak;
int partial_key=0;
bool force_partial=false;

void getkeys(fcd_t *fcd, table_t *tab) {

    unsigned short nkeys;
    unsigned short cdaoffset, ncomps, offset, k, c;
    unsigned char  *kda, *cda;
    int len;
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

                        ptr = ptr->next;
                        while ((len > 0) && (ptr != NULL)) {
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
            }
        }
        tab->keys = list2_append(tab->keys, (_key_t *)&key, sizeof(_key_t));
    }
    tab->keys = list2_first(tab->keys);

    if (dbg > 2) {
        for (ptr=tab->keys; ptr!=NULL; ptr=ptr->next) {
            _key_t *key = (_key_t *) ptr->buf;
            fprintf(flog, "%ld key %d %d %d\n", time(NULL), key->id, key->ncomps, key->ncols);
            for (c=0; c<key->ncols; c++) {
                fprintf(flog, "    %s\n", key->columns[c]->name);
            }
        }
    }
}

void adiciona_comp(unsigned char *record, _key_t key, int c, char *_op, char *where, char *order) {

    char aux[257], op[3];
    unsigned char  buf[257];
    column_t *col;
    int last;

    strcpy(op, _op);
    if (partial_key > 0) {
        strcpy(op, "=");
    }

    col = key.columns[c];
    memcpy(buf, record+col->offset, col->len);
    buf[col->len] = 0;
    if ((col->tp == 'n') && !buf[0]) {
        memset(buf, '0', col->len);
    }

    last = key.ncols - 1;
    if (partial_weak) {
        last--;
    }
    if (partial_key > 0) {
        last = partial_key - 1;
    }
    if (dbg > 2) {
        fprintf(flog, "getwhere [%s] [%s] partial=%d\n", where, col->name, partial_key);
    }

    if (c == last) {
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
        if (partial_key > 0) {
            for (c=partial_key+1; c<key.ncols; c++) {
                col = key.columns[c];
                strcat(order, ",");
                strcat(order, col->name);
            }
        }
        if (partial_weak) {
            col = key.columns[c+1];
            strcat(order, ",");
            strcat(order, col->name);
        }
        return;
    }

    if ((op[0] != '>') && (op[0] != '<')) {
        if (col->tp == 'n') {
            sprintf(aux, "%s %s %s and ", col->name, op, buf);
        } else {
            sprintf(aux, "%s %s '%s' and ", col->name, op, buf);
        }
        strcat(where, aux);
        strcat(order, col->name);
        strcat(order, ",");
        adiciona_comp(record, key, c+1, op, where, order);
        return;
    }

    if (col->tp == 'n') {
        sprintf(aux, "%s %c %s or ( %s = %s and ( ", col->name, op[0], buf, col->name, buf);
    } else {
        sprintf(aux, "%s %c '%s' or ( %s = '%s' and ( ", col->name, op[0], buf, col->name, buf);
    }
    strcat(where, aux);
    strcat(order, col->name);
    if (op[0] == '<') {
        strcat(order, " desc");
    }
    strcat(order, ",");
    adiciona_comp(record, key, c+1, op, where, order);
    strcat(where, " ) ) ");
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
            partial_key = table->partial_key;
            if (force_partial && !memcmp(table->name, "intori", 6) && (keyid == 1)) {
                partial_key = 2;
            }
            adiciona_comp(record, *key, 0, op, where, order);
            partial_key = 0;
            break;
        }
    }
}

int seq;

void adiciona_comp_prepared(table_t *tab, _key_t key, int c, char *where, char cmd, int keyid) {

    char aux[257];
    column_t *col;
    int last;

    col = key.columns[c];
    col->p = seq;
    last = key.ncols - 1;
    if (partial_weak) {
        last--;
    }
    //fprintf(flog, "getwhereprep [%s] [%s] %d %d %d\n", where, col->name, partial_weak, c, last);

    if (c == last) {
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
                tab->prms_random[keyid] = list2_append(tab->prms_random[keyid], col, sizeof(column_t));
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
            tab->prms_random[keyid] = list2_append(tab->prms_random[keyid], col, sizeof(column_t));
            break;
    }
    adiciona_comp_prepared(tab, key, c+1, where, cmd, keyid);

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
            tab->prms_random[keyid] = list2_free(tab->prms_random[keyid]);
            break;
    }
    strcpy(where, "");
    seq = ini;
    for (ptr=tab->keys, k=0; ptr!=NULL; ptr=ptr->next, k++) {
        if (k == keyid) {
            key = (_key_t *) ptr->buf;
            adiciona_comp_prepared(tab, *key, 0, where, cmd, keyid);
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
            tab->prms_random[keyid] = list2_first(tab->prms_random[keyid]);
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

