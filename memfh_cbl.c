#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pqfh.h"
#include "memfh.h"

#define MAX_HDRS 128

extern int dbg;

int nhdrs=0;
memfh_hdr_t *hdrs[128];
bool start=false;

void memfh_cbl_open_output(fcd_t *fcd, char *filename);

void memfh_cbl_open_input(fcd_t *fcd, char *filename) {

    for (int i=0; i<MAX_HDRS; i++) {
        if ((hdrs[i] != 0) && !strcmp(hdrs[i]->filename, filename)) {    
            memcpy(fcd->status, ST_OK, 2);    
            fcd->open_mode = 0;
            int fileid = (int) hdrs[i];
            hdrs[i]->open++;
            putint(fcd->file_id, fileid);
            return;
        }
    }

    memcpy(fcd->status, ST_FILE_NOT_FOUND, 2);
}

void memfh_cbl_open_io(fcd_t *fcd, char *filename) {

    for (int i=0; i<MAX_HDRS; i++) {
        if ((hdrs[i] != 0) && !strcmp(hdrs[i]->filename, filename)) {    
            memcpy(fcd->status, ST_OK, 2);    
            fcd->open_mode = 1;
            int fileid = (int) hdrs[i];
            hdrs[i]->open++;
            putint(fcd->file_id, fileid);
            return;
        }
    }

    memfh_cbl_open_output(fcd, filename);
}

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
        if (k == 0) {
            keys[k] = malloc((1 + ncomps * 2) * sizeof(int));
        } else {
            keys[k] = malloc((1 + (ncomps + 1) * 2) * sizeof(int));
        }
        keys[k][0] = ncomps;

        for (c=0; c<ncomps; c++) {

            // component definition area 
            cda = fcd->kdb + cdaoffset + (c * 10);
            offset = getint(cda + 2);
            len = getint(cda + 6);
            keys[k][1+c*2] = offset;
            keys[k][1+c*2+1] = len;

        }    

        if (k > 0) {
            keys[k][0] = ncomps + 1;
            keys[k][1+ncomps*2] = keys[0][1];
            keys[k][1+ncomps*2+1] = keys[0][2];
        }        
    }    

    hdr = memfh_open(filename, reclen, nkeys, keys);
    hdr->open++;
    fileid = (int) hdr;
    putint(fcd->file_id, fileid);

    memcpy(fcd->status, ST_OK, 2);    
    fcd->open_mode = 2;
    if (nhdrs == 0) {
        for (int i=0; i<MAX_HDRS; i++) {
            hdrs[i] = 0;
        }
    }    
    for (int i=0; i<MAX_HDRS; i++) {
        if (hdrs[i] == 0) {
            hdrs[i] = hdr;
            nhdrs++;
            break;
        }
    }    

    //fprintf(flog, "memfh_cbl_open_output %08x %s\n", fileid, hdr->filename);
}    

void memfh_cbl_write(fcd_t *fcd) {

    int fileid;
    memfh_hdr_t *hdr;

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_WRITE, 2);
        return;
    }

    fileid = getint(fcd->file_id);    
    hdr = (memfh_hdr_t *) fileid;

    //fprintf(flog, "memfh_cbl_write %08x %s %d\n", fileid, hdr->filename, hdr->count);

    for (int i=0; i<MAX_HDRS; i++) {
        if (hdrs[i] == hdr) {    
            memfh_write(hdr, (char *) fcd->record);
            memcpy(fcd->status, ST_OK, 2);    
            return;
        }
    }
    
    memcpy(fcd->status, ST_NOT_OPENED_WRITE, 2);    
}

void memfh_cbl_close(fcd_t *fcd) {

    int fileid;
    memfh_hdr_t *hdr;

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_ALREADY_CLOSED, 2);
        return;
    }

    fileid = getint(fcd->file_id);    
    hdr = (memfh_hdr_t *) fileid;

    //fprintf(flog, "memfh_cbl_close %08x %s %d\n", fileid, hdr->filename, hdr->count);

    for (int i=0; i<MAX_HDRS; i++) {
        if (hdrs[i] == hdr) {    
            hdr->open--;
            //memfh_idx_list(hdr);
            //memfh_list(hdr);
            if (hdr->open == 0) {    
                memfh_close(hdr);
                hdrs[i] = 0;
                nhdrs--;
            }
            memcpy(fcd->status, ST_OK, 2);    
            fcd->open_mode = 128;
            return;
        }
    }    
    
    memcpy(fcd->status, ST_ALREADY_CLOSED, 2);    
}

void memfh_cbl_start(fcd_t *fcd) {

    int fileid;
    short keyid;
    memfh_hdr_t *hdr;

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_READ, 2);
        return;
    }

    fileid = getint(fcd->file_id);    
    keyid = getshort(fcd->key_id);
    hdr = (memfh_hdr_t *) fileid;

    if (dbg > 2) {
        fprintf(flog, "memfh_cbl_start %08x %s %d %d\n", fileid, hdr->filename, keyid, hdr->count);
    }    
    if (memfh_start(hdr, (char *) fcd->record, keyid)) {
        memcpy(fcd->status, ST_OK, 2);    
        start = true;
    } else {
        memcpy(fcd->status, ST_EOF, 2);    
    }        
}

void memfh_cbl_next(fcd_t *fcd) {

    int fileid;
    short keyid;
    memfh_hdr_t *hdr;

    if (fcd->open_mode == 128) {
        memcpy(fcd->status, ST_NOT_OPENED_READ, 2);
        return;
    }

    fileid = getint(fcd->file_id);    
    keyid = getshort(fcd->key_id);    
    hdr = (memfh_hdr_t *) fileid;
    if (start) {
        memcpy(fcd->status, ST_OK, 2);    
        start = false;
        return;
    }

    //fprintf(flog, "memfh_cbl_next %08x %s %d\n", fileid, hdr->filename, hdr->count);
    if (memfh_next(hdr, (char *) fcd->record, keyid)) {
        memcpy(fcd->status, ST_OK, 2);    
    } else {
        memcpy(fcd->status, ST_EOF, 2);    
    }    
}

void memfh_cbl(unsigned short op, fcd_t *fcd, char *filename) {

    if (dbg > 1) {    
        fprintf(flog, "memfh_cbl %04x [%s]\n", op, filename);
    }    

    switch (op) {

        case OP_OPEN_INPUT:    
            memfh_cbl_open_input(fcd, filename);
            break;

        case OP_OPEN_IO:    
            memfh_cbl_open_io(fcd, filename);
            break;

        case OP_OPEN_OUTPUT:    
            memfh_cbl_open_output(fcd, filename);
            break;

        case OP_WRITE:    
            memfh_cbl_write(fcd);
            break;

        case OP_START_GE:    
            memfh_cbl_start(fcd);
            break;

        case OP_READ_NEXT:    
            memfh_cbl_next(fcd);
            break;

        case OP_CLOSE:    
            memfh_cbl_close(fcd);
            break;
    }        

    if (dbg > 1) {    
        fprintf(flog, "memfh_cbl %04x [%s] st=%c%c\n", op, filename, fcd->status[0], fcd->status[1]); 
    }    

}
