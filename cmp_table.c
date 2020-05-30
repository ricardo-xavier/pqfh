#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqfh.h"

extern int dbg;
extern fcd_t *fcd_open;
extern table_t *tab_open;

int reccmp(unsigned char *recbd, unsigned char *recisam, int reclen, table_t *tab) {
    funcao = _RECCMP;
    list2_t *ptr;
    column_t *col;    
    unsigned char buf_isam[MAX_REC_LEN+1];
    unsigned char buf_bd[MAX_REC_LEN+1];
    int c, i;
        
    for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

        col = (column_t *) ptr->buf;
        memcpy(buf_bd, recbd+col->offset, col->len);
        buf_bd[col->len] = 0;
        memcpy(buf_isam, recisam+col->offset, col->len);
        buf_isam[col->len] = 0;

        c = memcmp(buf_isam, buf_bd, col->len);
        if (c != 0) {
            if (col->tp == 's') {
                for (i=0; i<col->len; i++) {
                    if (buf_isam[i] == 0) {
                        buf_isam[i] = ' ';
                    }
                }        
                c = memcmp(buf_isam, buf_bd, col->len);
            } else {
                for (i=0; i<col->len; i++) {
                    if ((buf_isam[i] == 0) || (buf_isam[i] == ' ')) {
                        buf_isam[i] = '0';
                    }
                }        
                c = memcmp(buf_isam, buf_bd, col->len);
            }        
        }

        if ((dbg > 1) && (c != 0)) {
            fprintf(stderr, "cmp=%d\n", c);    
            fprintf(flog, "%s %c %d:%d,%d [%s]\n", col->name, col->tp, col->offset, col->len, col->dec, buf_isam);
            fprintf(flog, "%s %c %d:%d,%d [%s]\n", col->name, col->tp, col->offset, col->len, col->dec, buf_bd);
        }

        if (c != 0) {
            return c;
        }        

    }

    return 0;
}

void cmp_table(PGconn *conn, bool sync) {
    funcao = _CMP_TABLE;
    fcd_t *fcd_bd, *fcd_isam;
    unsigned char opcode[2], record1[MAX_REC_LEN+1], record2[MAX_REC_LEN+1];
    char logname[257];
    FILE *f;
    int c, kofs, k;
    short reclen;
    unsigned short cdaoffset, ncomps;
    unsigned char  *kda, *cda, st[2];
    int ofs[MAX_COMPS];
    int len[MAX_COMPS];
    char key_bd[257], key_isam[257];

    if ((fcd_open == NULL) || (fcd_open->isam == 'S')) {
        return;
    }
    fcd_bd = fcd_open;
    fcd_isam = malloc(sizeof(fcd_t));
    memcpy(fcd_isam, fcd_bd, sizeof(fcd_t));

    if (dbg > 0) {
        fprintf(flog, "%ld cmp [%s]\n", time(NULL), tab_open->name);
    }

    sprintf(logname, "%s_cmp.log", tab_open->name);
    if ((f = fopen(logname, "w")) == NULL) {
        free(fcd_isam);
        return;
    }

    fcd_isam->open_mode = 128;

    reclen = getshort(fcd_bd->rec_len);
    fcd_bd->record = (unsigned char *) record1;
    record1[reclen] = 0;
    fcd_isam->record = (unsigned char *) record2;
    record2[reclen] = 0;

    putshort(opcode, OP_OPEN_INPUT);
    EXTFH(opcode, fcd_isam);
    if (fcd_isam->status[0] == '9') {
        errorisam("cmp", opcode, fcd_isam);    
    }        
    if (dbg > 2) {
        fprintf(flog, "%ld cmp open isam %c%c %d\n\n", time(NULL), fcd_isam->status[0], fcd_isam->status[1], fcd_isam->status[1]);
    }

    kda = fcd_bd->kdb + 14;
    ncomps = getshort(kda + 0);
    cdaoffset = getshort(kda + 2);
    for (c=0; c<ncomps; c++) {
        cda = fcd_bd->kdb + cdaoffset + (c * 10);
        ofs[c] = getint(cda + 2);
        len[c] = getint(cda + 6);
    }


    memset(fcd_bd->record, 0, reclen);
    op_start(conn, fcd_bd, ">=");
    if (dbg > 2) {
        fprintf(flog, "%ld cmp start bd %c%c %d\n", time(NULL), fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
    }

    putshort(opcode, OP_START_GT);
    memset(fcd_isam->record, 0, reclen);
    EXTFH(opcode, fcd_isam);
    if (fcd_isam->status[0] == '9') {
        errorisam("cmp", opcode, fcd_isam);    
    }        
    if (dbg > 2) {
        fprintf(flog, "%ld cmp start isam %c%c %d\n\n", time(NULL), fcd_isam->status[0], fcd_isam->status[1], fcd_isam->status[1]);
    }

    op_next_prev(conn, fcd_bd, 'n');
    memcpy(st, fcd_bd->status, 2);
    if (dbg > 2) {
        fprintf(flog, "%ld cmp next bd %c%c %d\n", time(NULL), fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
    }

    putshort(opcode, OP_READ_NEXT);
    EXTFH(opcode, fcd_isam);
    if (fcd_isam->status[0] == '9') {
        errorisam("cmp", opcode, fcd_isam);    
    }        
    if (dbg > 2) {
        fprintf(flog, "%ld cmp next isam %c%c %d\n\n", time(NULL), fcd_isam->status[0], fcd_isam->status[1], fcd_isam->status[1]);
    }

    while (!memcmp(fcd_bd->status, ST_OK, 2) || !memcmp(fcd_isam->status, ST_OK, 2)) {

        if (!memcmp(fcd_bd->status, ST_OK, 2)) {
            kofs = 0;
            for (c=0; c<ncomps; c++) {
                memcpy(key_bd+kofs, fcd_bd->record+ofs[c], len[c]);
                kofs += len[c];
            } 
            key_bd[kofs] = 0;
            if (dbg > 2) {
                fprintf(flog, "cmp key_bd  =[%s]\n", key_bd);
            }
        }        

        if (!memcmp(fcd_isam->status, ST_OK, 2)) {
            kofs = 0;
            for (c=0; c<ncomps; c++) {
                memcpy(key_isam+kofs, fcd_isam->record+ofs[c], len[c]);
                kofs += len[c];
            } 
            key_isam[kofs] = 0;
            if (dbg > 2) {
                fprintf(flog, "cmp key_isam=[%s]\n", key_isam);
            }
        }        

        if (!memcmp(fcd_bd->status, ST_OK, 2) && !memcmp(fcd_isam->status, ST_OK, 2)) {

            // compara chaves
            k = memcmp(key_bd, key_isam, kofs);
            if (!k) {
                // chaves iguais - compara registros
                if (reccmp(fcd_bd->record, fcd_isam->record, reclen, tab_open)) {
                    // registros diferentes
                    fprintf(f, ">");
                    fwrite(fcd_bd->record, reclen, 1, f);
                    fprintf(f, "\n");
                    fprintf(f, "<");
                    fwrite(fcd_isam->record, reclen, 1, f);
                    fprintf(f, "\n");

                    if (sync) {
                        char tmp[MAX_REC_LEN+1];
                        memcpy(tmp, fcd_bd->record, reclen);
                        memcpy(fcd_bd->record, fcd_isam->record, reclen);
                        op_rewrite(conn, fcd_bd);
                        if (dbg > 2) {
                            fprintf(flog, "%ld cmp rewrite [%s] %c%c %d\n\n", time(NULL), key_bd, fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
                        }
                        memcpy(fcd_bd->status, st, 2);
                        memcpy(fcd_bd->record, tmp, reclen);
                    }
                }
                op_next_prev(conn, fcd_bd, 'n');
                memcpy(st, fcd_bd->status, 2);
                if (dbg > 2) {
                    fprintf(flog, "%ld cmp next bd %c%c %d\n", time(NULL), fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
                }
                EXTFH(opcode, fcd_isam);
                if (fcd_isam->status[0] == '9') {
                    errorisam("cmp", opcode, fcd_isam);    
                }        
                if (dbg > 2) {
                    fprintf(flog, "%ld cmp next isam %c%c %d\n\n", time(NULL), fcd_isam->status[0], fcd_isam->status[1], fcd_isam->status[1]);
                }
                continue;
            }

            if (k > 0) {
                // o registro existe apenas no arquivo 2(isam)
                fprintf(f, "-");
                fwrite(fcd_isam->record, reclen, 1, f);
                fprintf(f, "\n");
                if (sync) {
                    char tmp[MAX_REC_LEN+1];
                    memcpy(tmp, fcd_bd->record, reclen);
                    memcpy(fcd_bd->record, fcd_isam->record, reclen);
                    op_write(conn, fcd_bd);
                    if (dbg > 2) {
                        fprintf(flog, "%ld cmp write [%s] %c%c %d\n", time(NULL), key_isam, fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
                    }
                    memcpy(fcd_bd->status, st, 2);
                    memcpy(fcd_bd->record, tmp, reclen);
                }
                EXTFH(opcode, fcd_isam);
                if (fcd_isam->status[0] == '9') {
                    errorisam("cmp", opcode, fcd_isam);    
                }        
                if (dbg > 2) {
                    fprintf(flog, "%ld cmp next isam %c%c %d\n\n", time(NULL), fcd_isam->status[0], fcd_isam->status[1], fcd_isam->status[1]);
                }
                continue;
            }

            // o registro existe apenas no arquivo 1(banco)
            fprintf(f, "+");
            fwrite(fcd_bd->record, reclen, 1, f);
            fprintf(f, "\n");
            if (sync) {
                op_delete(conn, fcd_bd);
                if (dbg > 2) {
                    fprintf(flog, "%ld cmp delete [%s] %c%c %d\n", time(NULL), key_bd, fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
                }
                memcpy(fcd_bd->status, st, 2);
            }
            op_next_prev(conn, fcd_bd, 'n');
            memcpy(st, fcd_bd->status, 2);
            if (dbg > 2) {
                fprintf(flog, "%ld cmp next bd %c%c %d\n\n", time(NULL), fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
            }
            continue;

        } else if (!memcmp(fcd_bd->status, ST_OK, 2)) {
            // + - fim do arquivo 2(isam)
            fprintf(f, "+");
            fwrite(fcd_bd->record, reclen, 1, f);
            fprintf(f, "\n");
            if (sync) {
                op_delete(conn, fcd_bd);
                if (dbg > 2) {
                    fprintf(flog, "%ld cmp delete [%s] %c%c %d\n", time(NULL), key_bd, fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
                }
                memcpy(fcd_bd->status, st, 2);
            }
            op_next_prev(conn, fcd_bd, 'n');
            memcpy(st, fcd_bd->status, 2);
            if (dbg > 2) {
                fprintf(flog, "%ld cmp next bd %c%c %d\n\n", time(NULL), fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
            }
            continue;

        } else {
            // - - fim do arquivo 1(banco)
            fprintf(f, "-");
            fwrite(fcd_isam->record, reclen, 1, f);
            fprintf(f, "\n");
            if (sync) {
                char tmp[MAX_REC_LEN+1];
                memcpy(tmp, fcd_bd->record, reclen);
                memcpy(fcd_bd->record, fcd_isam->record, reclen);
                op_write(conn, fcd_bd);
                if (dbg > 2) {
                    fprintf(flog, "%ld cmp write [%s] %c%c %d\n", time(NULL), key_isam, fcd_bd->status[0], fcd_bd->status[1], fcd_bd->status[1]);
                }        
                memcpy(fcd_bd->status, st, 2);
                memcpy(fcd_bd->record, tmp, reclen);
            }
            EXTFH(opcode, fcd_isam);
            if (fcd_isam->status[0] == '9') {
                errorisam("cmp", opcode, fcd_isam);    
            }        
            if (dbg > 2) {
                fprintf(flog, "%ld cmp next isam %c%c %d\n", time(NULL), fcd_isam->status[0], fcd_isam->status[1], fcd_isam->status[1]);
            }
            continue;
        } 

    }
    fclose(f);
    free(fcd_isam);

}
