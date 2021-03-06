#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "memfh.h"

#ifndef PQFH
int dbg=0;
#define flog stderr
#else
extern int dbg;
#endif

memfh_hdr_t *memfh_open(char *filename, int reclen, int nkeys, int **keys) {

    memfh_hdr_t *hdr;

    if (dbg >= 3) {
        fprintf(flog, "===== memfh_open [%s] %d %d\n", filename, reclen, nkeys);
        for (int k=0; k<nkeys; k++) {
            fprintf(flog, "%d %d\n", k, keys[k][0]);    
            for (int c=0; c<keys[k][0]; c++) {
                fprintf(flog, "    %d:%d\n", keys[k][1+c*2], keys[k][1+c*2+1]);        
            }        
        }        
    }
    hdr = malloc(sizeof(memfh_hdr_t));
    hdr->filename = strdup(filename);
    hdr->count = 0;
    hdr->reclen = reclen;
    hdr->nkeys = nkeys;
    hdr->keys = keys;
    hdr->head = NULL;
    hdr->tail = NULL;

    hdr->idx = malloc(nkeys * sizeof(memfh_idx_t *));

    // aloca a pk
    hdr->idx[0] = malloc(sizeof(memfh_idx_t));
    hdr->idx[0]->tp = 0;
    hdr->idx[0]->n = 0;
    hdr->idx[0]->next = NULL;

    // as outras chaves serao alocadas sob demanda
    for (int k=1; k<nkeys; k++) {
        hdr->idx[k] = NULL;
    }    

    return hdr;        
}        

void memfh_close(memfh_hdr_t *hdr) {

    memfh_data_t *data, *next;

    if (hdr == NULL) {
        return;
    }    

    data = hdr->head;
    while (data != NULL) {
        if (data->record != NULL) {    
            free(data->record);
            data->record = NULL;
        }    
        next = data->next;
        free(data);    
        data = next;
    }

    if (hdr->idx != NULL) {
        for (int k=0; k<hdr->nkeys; k++) {
            if (hdr->idx[k] != NULL) {
                free(hdr->idx[k]);
                hdr->idx[k] = NULL;
            }
        }
        free(hdr->idx);
        hdr->idx = NULL;
    }    

    if (hdr->keys != NULL) {
        for (int k=0; k<hdr->nkeys; k++) {
            if (hdr->keys[k] != NULL) {
                free(hdr->keys[k]);
                hdr->keys[k] = NULL;
            }
        }
        free(hdr->keys);
        hdr->keys = NULL;
    }    

    if (hdr->filename != NULL) {
        free(hdr->filename);
        hdr->filename = NULL;
    }    
    free(hdr);
}        

void memfh_list(memfh_hdr_t *hdr) {

    memfh_data_t *data;
    int i=0;

    if (hdr == NULL) {
        return;
    }    

    data = hdr->head;
    while (data != NULL) {
        fprintf(flog, "%09d %08x [%s] %08x\n", ++i, (CAST) data, data->record, (CAST) data->next);    
        data = data->next;
    }

}        

void memfh_write(memfh_hdr_t *hdr, char *record) {

    memfh_data_t *data;
    char key[MEMFH_MAX_KEYLEN+1];
    int offset;

    data = malloc(sizeof(memfh_data_t));
    data->next = NULL;
    data->record = malloc(hdr->reclen + 1);
    memcpy(data->record, record, hdr->reclen);
    data->record[hdr->reclen] = 0;

    if (hdr->count == 0) {
        hdr->head = data;
        hdr->tail = data;
    } else {
        hdr->tail->next = data;
        hdr->tail = data;    
    }    

    // monta a chave
    offset = 0;
    for (int c=0; c<hdr->keys[0][0]; c++) {
        memcpy(key + offset, record + hdr->keys[0][1+c*2], hdr->keys[0][1+c*2+1]);
        offset += hdr->keys[0][1+c*2+1];
    }    
    key[offset] = 0;
    hdr->idx[0]->keylen = offset;
    memfh_idx_write(hdr, 0, key, data->record);

    hdr->count++;
}

bool memfh_read(memfh_hdr_t *hdr, char *record, bool update) {

    int depth;    
    char key[257], *ptr, *buf;
    memfh_idx_t *idx;

    // monta a chave
    int offset = 0;
    for (int c=0; c<hdr->keys[0][0]; c++) {
        memcpy(key + offset, record + hdr->keys[0][1+c*2], hdr->keys[0][1+c*2+1]);
        offset += hdr->keys[0][1+c*2+1];
    }    
    key[offset] = 0;

    int ret = memfh_idx_search(hdr, 0, key);
    if (ret == 0) {
        depth = hdr->depth[0];
        idx = hdr->path[0][depth];
        int pos = hdr->pos[0][depth];
        ptr = idx->buf;
        ptr = idx->buf + pos * (idx->keylen + sizeof(char *));
        memcpy(key, ptr, idx->keylen);
        key[idx->keylen] = 0;    
        memcpy(&buf, ptr+idx->keylen, sizeof(char *));
        if (!update) {
            memcpy(record, buf, hdr->reclen);
        } else {
            if (memcmp(buf, record, idx->keylen)) {
                fprintf(stderr, "erro update\n");
                exit(-1);
            }
            memcpy(buf, record, hdr->reclen);
        }
        if (dbg >= 3) {
            fprintf(flog, "%04d [%s] [%s]\n", pos, key, buf);
        }
    }
    return ret == 0;    
}

bool memfh_start(memfh_hdr_t *hdr, char *record, int k) {

    int depth;    
    char key[257], *ptr, *buf;
    memfh_idx_t *idx;

    if ((k > 0) && (hdr->idx[k] == NULL)) {
        memfh_idx_create(hdr, k);        
    }

    // monta a chave
    int offset = 0;
    for (int c=0; c<hdr->keys[k][0]; c++) {
        memcpy(key + offset, record + hdr->keys[k][1+c*2], hdr->keys[k][1+c*2+1]);
        offset += hdr->keys[k][1+c*2+1];
    }    
    key[offset] = 0;

    int ret = memfh_idx_search(hdr, k, key);

    if (ret <= 0) {
        depth = hdr->depth[k];
        idx = hdr->path[k][depth];
        ptr = idx->buf;
        memcpy(key, ptr, idx->keylen);
        key[idx->keylen] = 0;    
        memcpy(&buf, ptr+idx->keylen, sizeof(char *));
        memcpy(record, buf, hdr->reclen);
        if (dbg >= 3) {
            fprintf(flog, "%04d [%s] [%s]\n", hdr->pos[k][depth], key, buf);
        }
        return true;
    }
    return false;
}

bool memfh_next(memfh_hdr_t *hdr, char *record, int k) {

    char key[257], *ptr, *buf;
    int depth = hdr->depth[k];
    memfh_idx_t *idx = hdr->path[k][depth];
    key[idx->keylen] = 0;    
    bool ret = memfh_idx_next(hdr, idx, k);

    if (ret) {
        depth = hdr->depth[k];
        idx = hdr->path[k][depth];
        ptr = idx->buf + hdr->pos[k][depth] * (idx->keylen + sizeof(char *));
        memcpy(key, ptr, idx->keylen);
        memcpy(&buf, ptr+idx->keylen, sizeof(char *));
        memcpy(record, buf, hdr->reclen);
        if (dbg >= 3) {
            fprintf(flog, "%04d [%s] [%s]\n", hdr->pos[k][depth], key, buf);
        }
    }
    return ret;    
}

extern int num_writes;
extern int num_reads;

#ifndef PQFH
int teste_bairro(int argc, char *argv[]) {

    memfh_hdr_t *hdr;
    int **keys = malloc(7 * sizeof(int *));

    // cod
    keys[0] = malloc((1 + 1 * 2) * sizeof(int));
    keys[0][0] = 1;
    keys[0][1] = 0;
    keys[0][2] = 7;

    keys[4] = malloc((1 + 3 * 2) * sizeof(int));
    keys[4][0] = 3;
    keys[4][1] = 51;
    keys[4][2] = 15;
    keys[4][3] = 7;
    keys[4][4] = 35;
    keys[4][5] = 0;
    keys[4][6] = 7;
    hdr = memfh_open("teste", 129, 7, keys);    

    int n = 999999999;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    FILE *f = fopen("bairro.log", "r");
    char buf[257];
    char *record;
    int i = 0;
    num_writes = 0;
    while (fgets(buf, 257, f) != NULL) {
        record = buf + 20;
        record[129] = 0;
        //fprintf(flog, "[%s]\n", record);
        memfh_write(hdr, record);
        if (++i == n) {
            break;
        }
    }
    fclose(f);

    num_reads = 0;
    memfh_idx_list(hdr);
    fprintf(flog, "%d %d %d\n", i, num_writes, num_reads);

    memset(buf, 0, 257);
    //memcpy(buf, "                                                   YPIRANGA", 59);
    num_reads = 0;
    num_writes = 0;
    memfh_start(hdr, buf, 4);
    fprintf(flog, "%d %d %d\n", i, num_writes, num_reads);

    return 0;
}    

int teste_itens(int argc, char *argv[]) {

    memfh_hdr_t *hdr;
    int **keys = malloc(1 * sizeof(int *));

    keys[0] = malloc((1 + 1 * 2) * sizeof(int));
    keys[0][0] = 1;
    keys[0][1] = 0;
    keys[0][2] = 6;

    hdr = memfh_open("totitem", 19, 1, keys);    

    int n = 999999999;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    FILE *f = fopen("itens.log", "r");
    char buf[257];
    char record[257];
    int i = 0;
    int ins = 0;
    int upd = 0;
    num_writes = 0;
    record[19] = 0;
    while (fgets(buf, 257, f) != NULL) {
        memcpy(record, buf, 19);
        fprintf(stderr, "+%d %d %d [%s]\n", i, ins, upd, record);
        bool existe = memfh_read(hdr, record, false);
        if (existe) {
            long vlr_atual = atol(record+6);    
            long vlr_adicionar = atol(buf+6);    
            sprintf(buf+6, "%013ld", vlr_atual + vlr_adicionar);
            memfh_read(hdr, buf, true);
            upd++;
        } else {
            memfh_write(hdr, record);
            ins++;
        }
        if (++i == n) {
            break;
        }
    }
    fclose(f);

    memset(record, 0, 257);
    memfh_start(hdr, record, 0);

    for (;;) {
        fprintf(stderr, "[%s]\n", record);

        if (!memfh_next(hdr, record, 0)) {
            break;
        }
    }

    return 0;
}    

int main(int argc, char *argv[]) {
    teste_itens(argc, argv);
    return 0;
}
#endif

