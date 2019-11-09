#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "memfh.h"

memfh_hdr_t *memfh_open(char *filename, int reclen, int nkeys, int **keys) {

    memfh_hdr_t *hdr;

    fprintf(stderr, "===== memfh_open [%s] %d %d\n", filename, reclen, nkeys);
    for (int k=0; k<nkeys; k++) {
        fprintf(stderr, "%d %d\n", k, keys[k][0]);    
        for (int c=0; c<keys[k][0]; c++) {
            fprintf(stderr, "    %d:%d\n", keys[k][1+c*2], keys[k][1+c*2+1]);        
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
        fprintf(stderr, "%09d %08x [%s] %08x\n", ++i, (int) data, data->record, (int) data->next);    
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
    memfh_idx_write(hdr, 0, offset, key, data->record);

    hdr->count++;
}

/*
int main(int argc, char *argv[]) {
    memfh_hdr_t *hdr;
    hdr = memfh_open("teste", 13, 3);    
    memfh_write(hdr, "001RICARDO   ");
    memfh_write(hdr, "002RONALDO   ");
    memfh_write(hdr, "003BRENO     ");
    memfh_write(hdr, "004CRISTIANO ");
    memfh_list(hdr);
    memfh_close(hdr);
    return 0;
}    
*/
