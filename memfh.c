#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef struct memfh_data_s {
    struct memfh_data_s *next;
    char *record;
} memfh_data_t;    

typedef struct memfh_hdr_s {
    char *filename;    
    int   count;
    int   reclen;
    int   keylen;
    memfh_data_t *head;
    memfh_data_t *tail;
} memfh_hdr_t;    

memfh_hdr_t *memfh_open(char *filename, int reclen, int keylen) {

    memfh_hdr_t *hdr;

    hdr = malloc(sizeof(memfh_hdr_t));
    hdr->count = 0;
    hdr->reclen = reclen;
    hdr->keylen = keylen;
    hdr->head = NULL;
    hdr->tail = NULL;

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
