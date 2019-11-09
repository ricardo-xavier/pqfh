#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "memfh.h"

memfh_idx_t *path[MEMFH_MAX_DEPTH];
int depth;
int pos;

void memfh_idx_list(memfh_hdr_t *hdr, memfh_idx_t *idx, int keylen) {

    char *ptr, key[257], *record;

    fprintf(stderr, "idx n=%d %08x %08x\n", idx->n, (int) idx, (int) idx->next);
    ptr = idx->buf;
    for (int i=0; i<idx->n; i++) {
        memcpy(key, ptr, keylen);
        key[keylen] = 0;    
        if (idx->tp == 0) {
            memcpy(&record, ptr+keylen, sizeof(char *));
            fprintf(stderr, "%d [%s] [%s]\n", i, key, record);
        } else {
            memfh_idx_t *page;
            memcpy(&page, ptr+keylen, sizeof(memfh_idx_t *));
            fprintf(stderr, "%d [%s] %08x\n", i, key, (int) page);
        }        
        ptr += (keylen + sizeof(char *));
    }        
}

bool memfh_idx_search_page(memfh_idx_t *idx, int keylen, char *key) {

    char *ptr = idx->buf;   
    memfh_idx_t *next;

    fprintf(stderr, "memfh_idx_search_page %d [%s] %d %d\n", keylen, key, idx->tp, idx->n);    

    for (int i=0; i<idx->n; i++) {
        int c = memcmp(key, ptr, keylen);
        if (c == 0) {
            pos = i;
            if (idx->tp == 0) {
                return true;
            } else {
                pos = -1;
                memcpy(&next, ptr+keylen, sizeof(memfh_idx_t *));
                fprintf(stderr, "push1 %08x\n", (int) next);
                path[depth] = next;
                depth++;
                return memfh_idx_search_page(next, keylen, key);
            }        
        }
        if (c < 0) {
            pos = i;
            if (idx->tp == 0) {
                return false;
            } else {
                pos = -1;
                memcpy(&next, ptr+keylen, sizeof(memfh_idx_t *));
                fprintf(stderr, "push2 %08x\n", (int) next);
                path[depth] = next;
                depth++;
                return memfh_idx_search_page(next, keylen, key);
            }
        }    
    }        

    if (idx->tp == 0) {
        pos = idx->n;
        return false;

    } else {    
        pos = -1;
        next = idx->next;
        fprintf(stderr, "push3 %08x\n", (int) next);
        path[depth] = next;
        depth++;
        return memfh_idx_search_page(next, keylen, key);
    }    
}

bool memfh_idx_search(memfh_hdr_t *hdr, int k, int keylen, char *key) {

    memfh_idx_t *idx = hdr->idx[k];
    path[0] = idx;
    pos = -1;    
    depth = 1;    
    return memfh_idx_search_page(idx, keylen, key);

}

void memfh_idx_write(memfh_hdr_t *hdr, int k, int keylen, char *key, char *record) {

    fprintf(stderr, "memfh_idx_write %d %d [%s]\n", k, keylen, key);
    bool exists = memfh_idx_search(hdr, k, keylen, key);
    if (exists) {
        exit(-1);    
    }        

    memfh_idx_t *idx = path[depth-1];
    fprintf(stderr, "depth=%d pos=%d/%d\n", depth, pos, idx->n);

    if (pos < idx->n) {    
        fprintf(stderr, "TODO abrir espaco\n");
        exit(-1);
    }        

    char *ptr = idx->buf + pos * (keylen + sizeof(char *));
    memcpy(ptr, key, keylen);
    memcpy(ptr+keylen, &record, sizeof(char *));
    idx->n++;

    if (idx->n < MEMFH_MAX_ITEMS(keylen)) {
        memfh_idx_list(hdr, idx, keylen);

    } else {

        int n1 = idx->n / 2;
        int n2 = idx->n - n1;

        idx->n = n1;
        memfh_idx_t *idx2 = malloc(sizeof(memfh_idx_t));
        idx2->tp = 0;
        idx2->next = NULL;
        idx2->n = n2;

        char *ptr1 = idx->buf + n1 * (keylen + sizeof(char *));
        char *ptr2 = idx2->buf;

        for (int i=0; i<n2; i++) {
            memcpy(ptr2, ptr1, keylen + sizeof(char *));        
            ptr1 += (keylen + sizeof(char *));
            ptr2 += (keylen + sizeof(char *));
        }
        memfh_idx_list(hdr, idx, keylen);
        memfh_idx_list(hdr, idx2, keylen);

        if (depth == 1) {    
            memfh_idx_t *parent = malloc(sizeof(memfh_idx_t));
            parent->tp = 1;
            parent->n = 1;
            ptr2 = idx2->buf;
            memcpy(parent->buf, ptr2, keylen);
            memcpy(parent->buf + keylen, &idx, sizeof(memfh_idx_t *));
            parent->next = idx2;

            hdr->idx[k] = parent;
            memfh_idx_list(hdr, parent, keylen);

        } else {
            memfh_idx_t *parent = path[depth-2];
            if (parent->next == idx) {
                // a pagina que quebrou era a ultima do pai    
                if (parent->n < MEMFH_MAX_ITEMS(keylen)) {
                    // tem espaco para inserir no pai    
                    ptr2 = idx2->buf;
                    memcpy(parent->buf + parent->n * (keylen + sizeof(memfh_idx_t)), ptr2, keylen);
                    memcpy(parent->buf + parent->n * (keylen + sizeof(memfh_idx_t)) + keylen, &idx, sizeof(memfh_idx_t *));
                    parent->next = idx2;
                    parent->n++;

                } else {
                    fprintf(stderr, "TODO a pagina que quebrou era a ultima do pai, mas o pai estava cheio\n");    
                    exit(-1);
                }        

            } else {
                fprintf(stderr, "TODO a pagina que quebrou nao era a ultima do pai\n");    
                exit(-1);
            }
                
        }    
    }        

}
