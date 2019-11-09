#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "memfh.h"

#define MAX_HDRS 128

int nhdrs=0;
memfh_hdr_t *hdrs[128];

void memfh_cbl_open_output(fcd_t *fcd, char *filename) {

    short nkeys, k, ncomps, c, cdaoffset, reclen;
    int offset, len, fileid;
    unsigned char  *kda, *cda;
    memfh_hdr_t *hdr;
    int **keys;

    reclen = getshort(fcd->rec_len);

    nkeys = getshort(fcd->kdb + 6);
    keys = malloc(nkeys * sizeof(int *));

    for (k=0; k<nkeys; k++) {

        // key definition area
        kda = fcd->kdb + 14 + (k * 16);
        ncomps = getshort(kda + 0);
        cdaoffset = getshort(kda + 2);
        keys[k] = malloc((1 + ncomps * 2) * sizeof(int));
        keys[k][0] = ncomps;

        for (c=0; c<ncomps; c++) {

            // component definition area 
            cda = fcd->kdb + cdaoffset + (c * 10);
            offset = getint(cda + 2);
            len = getint(cda + 6);
            keys[k][1+c*2] = offset;
            keys[k][1+c*2+1] = len;

        }    
    }    

    hdr = memfh_open(filename, reclen, nkeys, keys);
    fileid = (int) hdr;
    putint(fcd->file_id, fileid);

    memcpy(fcd->status, ST_OK, 2);    
    if (nhdrs == 0) {
        for (int i=0; i<MAX_HDRS; i++) {
            hdrs[i] = 0;
        }
    }    
    for (int i=0; i<MAX_HDRS; i++) {
        if (hdrs[i] == 0) {
            hdrs[i] = hdr;
            fprintf(stderr, "memfh_cbl_open i=%d %08x %d\n", i, (int) hdrs[i], hdr->reclen);
            nhdrs++;
            break;
        }
    }    

}    

void memfh_cbl_write(fcd_t *fcd) {

    int fileid;
    memfh_hdr_t *hdr;

    fileid = getint(fcd->file_id);    
    hdr = (memfh_hdr_t *) fileid;

    fprintf(stderr, "memfh_cbl_write %08x %d\n", fileid, nhdrs);

    for (int i=0; i<MAX_HDRS; i++) {
        if (hdrs[i] == hdr) {    
            memfh_write(hdr, (char *) fcd->record);
            fprintf(stderr, "memfh_cbl_write count=%d\n", hdr->count);
            memcpy(fcd->status, ST_OK, 2);    
            return;
        }
    }
    
    memcpy(fcd->status, ST_NOT_OPENED_WRITE, 2);    
}

void memfh_cbl_close(fcd_t *fcd) {

    int fileid;
    memfh_hdr_t *hdr;

    fileid = getint(fcd->file_id);    
    hdr = (memfh_hdr_t *) fileid;

    fprintf(stderr, "memfh_cbl_close %08x %d\n", fileid, nhdrs);

    for (int i=0; i<MAX_HDRS; i++) {
        fprintf(stderr, "memfh_cbl_close i=%d %08x\n", i, (int) hdrs[i]);
        if (hdrs[i] == hdr) {    
            fprintf(stderr, "memfh_cbl_close count=%d\n", hdr->count);
            memfh_list(hdr);
            //TODO criar um comando para fechar no pqfh - nao pode fechar no primeiro close
            /*
            memfh_close(hdr);
            hdrs[i] = 0;
            nhdrs--;
            */
            memcpy(fcd->status, ST_OK, 2);    
            return;
        }
    }    
    
    memcpy(fcd->status, ST_ALREADY_CLOSED, 2);    
}

void memfh_cbl(unsigned short op, fcd_t *fcd, char *filename) {

    fprintf(stderr, "memfh_cbl %04x [%s]\n", op, filename);

    switch (op) {

        case OP_OPEN_INPUT:    
        case OP_OPEN_IO:    
            break;

        case OP_OPEN_OUTPUT:    
            memfh_cbl_open_output(fcd, filename);
            break;

        case OP_WRITE:    
            memfh_cbl_write(fcd);
            break;

        case OP_CLOSE:    
            memfh_cbl_close(fcd);
            break;
    }        

}
