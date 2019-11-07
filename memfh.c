#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#define MEMFH_PAGELEN 4096

#define MEMFH_MAX_ITEMS(keylen) ((MEMFH_PAGELEN - 8) / (keylen + sizeof(char *)))

typedef struct memfh_data_s {
    struct memfh_data_s *next;
    char *record;
} memfh_data_t;    

typedef struct memfh_idx_s {
    char tp;
    char filler[3];
    int  n;    
    char buf[MEMFH_PAGELEN-8];
} memfh_idx_t;    

typedef struct memfh_hdr_s {
    char *filename;    
    int   count;
    int   reclen;
    int   keylen;
    memfh_data_t *head;
    memfh_data_t *tail;
    memfh_idx_t *idx;
} memfh_hdr_t;    

memfh_hdr_t *memfh_open(char *filename, int reclen, int keylen) {

    memfh_hdr_t *hdr;

    hdr = malloc(sizeof(memfh_hdr_t));
    hdr->filename = strdup(filename);
    hdr->count = 0;
    hdr->reclen = reclen;
    hdr->keylen = keylen;
    hdr->head = NULL;
    hdr->tail = NULL;
    hdr->idx = malloc(sizeof(memfh_idx_t));
    hdr->idx->tp = 0;
    hdr->idx->n = 0;

    return hdr;        
}        

void memfh_close(memfh_hdr_t *hdr) {

    memfh_data_t *data, *next;

    data = hdr->head;
    while (data != NULL) {
        free(data->record);
        next = data->next;
        free(data);    
        data = next;
    }

    free(hdr->idx);
    free(hdr->filename);
    free(hdr);
}        

void memfh_list(memfh_hdr_t *hdr) {

    memfh_data_t *data;
    int i=0;

    data = hdr->head;
    while (data != NULL) {
        fprintf(stderr, "%09d %08x [%s] %08x\n", ++i, (int) data, data->record, (int) data->next);    
        data = data->next;
    }

}        

void memfh_idx_list(memfh_hdr_t *hdr, memfh_idx_t *idx) {

    char *ptr, key[257], *record;

    fprintf(stderr, "idx n=%d\n", idx->n);    
    ptr = idx->buf;
    for (int i=0; i<idx->n; i++) {
        memcpy(key, ptr, hdr->keylen);
        key[hdr->keylen] = 0;    
        memcpy(&record, ptr+hdr->keylen, sizeof(char *));
        fprintf(stderr, "%d [%s] [%s]\n", i, key, record);
        ptr += (hdr->keylen + sizeof(char *));
    }        
}

void memfh_idx_write(memfh_hdr_t *hdr, char *key, char *record) {

    memfh_idx_t *idx;
    char *ptr;

    idx = hdr->idx;

    if (idx->tp == 0) {
        // folha
        if (idx->n < MEMFH_MAX_ITEMS(hdr->keylen)) {
            // procura a posicao de insercao    
            ptr = idx->buf;   
            for (int i=0; i<idx->n; i++) {
                if (memcmp(key, ptr, hdr->keylen) < 0) {
                    fprintf(stderr, "TODO abrir espaco\n");
                    exit(-1);
                }        
                ptr += (hdr->keylen + sizeof(char *));
            }
            memcpy(ptr, key, hdr->keylen);
            memcpy(ptr+hdr->keylen, &record, sizeof(char *));
            idx->n++;
            memfh_idx_list(hdr, idx);

        } else {
            fprintf(stderr, "TODO pagina cheia\n");
            exit(-1);

        }        
    } else {
        fprintf(stderr, "TODO no interno\n");
        exit(-1);
    }    

}

void memfh_write(memfh_hdr_t *hdr, char *record) {

    memfh_data_t *data;

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

    //TODO montar chave
    memfh_idx_write(hdr, data->record, data->record);

    hdr->count++;
}

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
