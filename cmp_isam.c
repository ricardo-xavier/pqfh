#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;
extern fcd_t *fcd_open;
extern char mode;

void cmp_isam(PGconn *conn, char *filename2) {

    fcd_t *fcd1, *fcd2;
    unsigned char opcode[2], record1[MAX_REC_LEN+1], record2[MAX_REC_LEN+1];
    char filename1[257];
    char logname[257], *p;
    FILE *f;
    int c, kofs, k;
    short reclen;
    unsigned short cdaoffset, ncomps;
    unsigned char  *kda, *cda;
    int ofs[MAX_COMPS];
    int len[MAX_COMPS];
    char key1[257], key2[257];

    if (fcd_open == NULL) {
        return;
    }
    if (fcd_open->isam != 'S' && mode != 'I') {
        return;
    }
    fcd1 = fcd_open;
    fcd1->ignore_lock |= 0x01;
    fcd2 = malloc(sizeof(fcd_t));
    memcpy(fcd2, fcd1, sizeof(fcd_t));

    memcpy(filename1, fcd1->file_name, strlen(filename2));
    filename1[strlen(filename2)] = 0;

    if (dbg > 0) {
        fprintf(flog, "%ld cmp_isam [%s] [%s]\n", time(NULL), filename1, filename2);
    }

    if ((p = strrchr(filename1, '/')) == NULL) {
        sprintf(logname, "%s_cmp.log", filename1);
    } else {
        sprintf(logname, "%s_cmp.log", p+1);
    }
    if ((f = fopen(logname, "w")) == NULL) {
        free(fcd2);
        return;
    }

    fcd2->open_mode = 128;
    fcd2->ignore_lock |= 0x01;
    fcd2->file_name = malloc(strlen(filename2) + 1);
    memcpy(fcd2->file_name, filename2, strlen(filename2));
    putshort(fcd2->file_name_len, strlen(filename2));

    reclen = getshort(fcd1->rec_len);
    fcd1->record = (unsigned char *) record1;
    record1[reclen] = 0;
    fcd2->record = (unsigned char *) record2;
    record2[reclen] = 0;

    putshort(opcode, OP_OPEN_INPUT);
    EXTFH(opcode, fcd2);
    if (dbg > 2) {
        fprintf(flog, "%ld cmpisam open [%s] %c%c %d\n", time(NULL), filename2, fcd2->status[0], fcd2->status[1], fcd2->status[1]);
    }

    kda = fcd1->kdb + 14 ;
    ncomps = getshort(kda + 0);
    cdaoffset = getshort(kda + 2);
    for (c=0; c<ncomps; c++) {
        cda = fcd1->kdb + cdaoffset + (c * 10);
        ofs[c] = getint(cda + 2);
        len[c] = getint(cda + 6);
    }

    putshort(opcode, OP_START_GT);

    memset(fcd1->record, 0, reclen);
    EXTFH(opcode, fcd1);
    if (dbg > 2) {
        fprintf(flog, "%ld cmpisam start [%s] %c%c %d\n", time(NULL), filename1, fcd1->status[0], fcd1->status[1], fcd1->status[1]);
    }

    memset(fcd2->record, 0, reclen);
    EXTFH(opcode, fcd2);
    if (dbg > 2) {
        fprintf(flog, "%ld cmpisam start [%s] %c%c %d\n", time(NULL), filename2, fcd2->status[0], fcd2->status[1], fcd2->status[1]);
    }

    putshort(opcode, OP_READ_NEXT);

    EXTFH(opcode, fcd1);
    if (dbg > 2) {
        fprintf(flog, "%ld cmpisam next [%s] %c%c %d\n", time(NULL), filename1, fcd1->status[0], fcd1->status[1], fcd1->status[1]);
    }

    EXTFH(opcode, fcd2);
    if (dbg > 2) {
        fprintf(flog, "%ld cmpisam next [%s] %c%c %d\n", time(NULL), filename2, fcd2->status[0], fcd2->status[1], fcd2->status[1]);
    }

    while (!memcmp(fcd1->status, ST_OK, 2) || !memcmp(fcd2->status, ST_OK, 2)) {

        if (!memcmp(fcd1->status, ST_OK, 2) && !memcmp(fcd2->status, ST_OK, 2)) {

            // compara chaves
            kofs = 0;
            for (c=0; c<ncomps; c++) {
                memcpy(key1+kofs, fcd1->record+ofs[c], len[c]);
                memcpy(key2+kofs, fcd2->record+ofs[c], len[c]);
                kofs += len[c];
            } 
            key1[kofs] = 0;
            key2[kofs] = 0;
            if (dbg > 2) {
                fprintf(flog, "cmpisam key1=[%s]\n", key1);
                fprintf(flog, "cmpisam key2=[%s]\n", key2);
            }

            k = memcmp(key1, key2, kofs);
            if (!k) {
                // chaves iguais - compara registros
                if (memcmp(fcd1->record, fcd2->record, reclen)) {
                    // registros diferentes
                    fprintf(f, "<");
                    fwrite(fcd2->record, reclen, 1, f);
                    fprintf(f, "\n");
                    fprintf(f, ">");
                    fwrite(fcd1->record, reclen, 1, f);
                    fprintf(f, "\n");
                }
                EXTFH(opcode, fcd1);
                if (dbg > 2) {
                    fprintf(flog, "%ld cmpisam next [%s] %c%c %d\n", time(NULL), filename1, fcd1->status[0], fcd1->status[1], fcd1->status[1]);
                }
                EXTFH(opcode, fcd2);
                if (dbg > 2) {
                    fprintf(flog, "%ld cmpisam next [%s] %c%c %d\n", time(NULL), filename2, fcd2->status[0], fcd2->status[1], fcd2->status[1]);
                }
                continue;
            }

            if (k > 0) {
                // o registro existe apenas no arquivo 2
                fprintf(f, "-");
                fwrite(fcd2->record, reclen, 1, f);
                fprintf(f, "\n");
                EXTFH(opcode, fcd2);
                if (dbg > 2) {
                    fprintf(flog, "%ld cmpisam next [%s] %c%c %d\n", time(NULL), filename2, fcd2->status[0], fcd2->status[1], fcd2->status[1]);
                }
                continue;
            }

            // o registro existe apenas no arquivo 1
            fprintf(f, "+");
            fwrite(fcd1->record, reclen, 1, f);
            fprintf(f, "\n");
            EXTFH(opcode, fcd1);
            if (dbg > 2) {
                fprintf(flog, "%ld cmpisam next [%s] %c%c %d\n", time(NULL), filename1, fcd1->status[0], fcd1->status[1], fcd1->status[1]);
            }
            continue;

        } else if (!memcmp(fcd1->status, ST_OK, 2)) {
            // + - fim do arquivo 2
            fprintf(f, "+");
            fwrite(fcd1->record, reclen, 1, f);
            fprintf(f, "\n");
            EXTFH(opcode, fcd1);
            if (dbg > 2) {
                fprintf(flog, "%ld cmpisam next [%s] %c%c %d\n", time(NULL), filename1, fcd1->status[0], fcd1->status[1], fcd1->status[1]);
            }
            continue;

        } else {
            // - - fim do arquivo 1
            fprintf(f, "-");
            fwrite(fcd2->record, reclen, 1, f);
            fprintf(f, "\n");
            EXTFH(opcode, fcd2);
            if (dbg > 2) {
                fprintf(flog, "%ld cmpisam next [%s] %c%c %d\n", time(NULL), filename2, fcd2->status[0], fcd2->status[1], fcd2->status[1]);
            }
            continue;
        } 

    }
    fclose(f);
    free(fcd2->file_name);
    free(fcd2);

}
