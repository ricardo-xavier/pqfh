#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "memfh.h"

memfh_hdr_t *memfh_open(char *filename, int reclen, int nkeys, int **keys) {

    memfh_hdr_t *hdr;

    /*
    fprintf(stderr, "===== memfh_open [%s] %d %d\n", filename, reclen, nkeys);
    for (int k=0; k<nkeys; k++) {
        fprintf(stderr, "%d %d\n", k, keys[k][0]);    
        for (int c=0; c<keys[k][0]; c++) {
            fprintf(stderr, "    %d:%d\n", keys[k][1+c*2], keys[k][1+c*2+1]);        
        }        
    }        
    */
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
        fprintf(stderr, "%09d %08x [%s] %08x\n", ++i, (CAST) data, data->record, (CAST) data->next);    
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
        memcpy(key, record + hdr->keys[0][1+c*2], hdr->keys[0][1+c*2+1]);
        offset += hdr->keys[0][1+c*2+1];
    }    
    key[offset] = 0;
    hdr->idx[0]->keylen = offset;
    memfh_idx_write(hdr, 0, key, data->record);

    hdr->count++;
}

bool memfh_start(memfh_hdr_t *hdr, char *record, int k) {

    int depth;    
    char key[257], *ptr, *buf;
    memfh_idx_t *idx;

    if ((k > 0) && (hdr->idx[k] == NULL)) {
        memfh_idx_create(hdr, k);        
    }

    bool ret = memfh_idx_first(hdr, k);

    if (ret) {
        depth = hdr->depth[k];
        idx = hdr->path[k][depth];
        ptr = idx->buf;
        memcpy(key, ptr, idx->keylen);
        key[idx->keylen] = 0;    
        memcpy(&buf, ptr+idx->keylen, sizeof(char *));
        memcpy(record, buf, hdr->reclen);
        //fprintf(stderr, "%04d [%s] [%s]\n", hdr->pos[0][depth], key, buf);
    }
    return ret;    
}

bool memfh_next(memfh_hdr_t *hdr, char *record) {

    char key[257], *ptr, *buf;
    int depth = hdr->depth[0];
    memfh_idx_t *idx = hdr->path[0][depth];
    key[idx->keylen] = 0;    
    bool ret = memfh_idx_next(hdr, idx, 0);

    if (ret) {
        depth = hdr->depth[0];
        idx = hdr->path[0][depth];
        ptr = idx->buf + hdr->pos[0][depth] * (idx->keylen + sizeof(char *));
        memcpy(key, ptr, idx->keylen);
        memcpy(&buf, ptr+idx->keylen, sizeof(char *));
        memcpy(record, buf, hdr->reclen);
        //fprintf(stderr, "%04d [%s] [%s]\n", hdr->pos[0][depth], key, buf);
    }
    return ret;    
}

/*
int main(int argc, char *argv[]) {

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
    while (fgets(buf, 257, f) != NULL) {
        record = buf + 20;
        record[129] = 0;
        //fprintf(stderr, "[%s]\n", record);
        memfh_write(hdr, record);
        if (++i == n) {
            break;
        }
    }
    fclose(f);

    //memfh_idx_list(hdr);

    memset(buf, 0, 257);
    memfh_start(hdr, buf, 4);

    return 0;
}    
*/
