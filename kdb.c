#include <stdlib.h>
#include "pqfh.h"

extern int dbg;

void kdb(fcd_t *fcd, unsigned int *offset, unsigned int *len) {

    unsigned short keyid, cdaoffset, c, ncomps;
    unsigned char  *kda, *cda;

    keyid = getshort(fcd->key_id);

    // key definition area
    kda = fcd->kdb + 14 + (keyid * 16);
    ncomps = getshort(kda + 0);
    cdaoffset = getshort(kda + 2);

    // component definition area 
    if (ncomps > 1) {
        fprintf(stderr, "pendente: chave com mais de um componente\n");
        exit(-1);
    }

    c = 0;
    cda = fcd->kdb + cdaoffset + (c * 10);

    *offset = getint(cda + 2);
    *len = getint(cda + 6);
}

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
                if (col->offset == offset) {
                    while (len > 0) {
                        col = (column_t *) ptr->buf;
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
