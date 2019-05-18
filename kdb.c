#include <stdlib.h>
#include "pqfh.h"

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
