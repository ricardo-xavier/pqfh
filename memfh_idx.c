#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "memfh.h"

bool memfh_idx_first(memfh_hdr_t *hdr, int k) {

    memfh_idx_t *idx = hdr->idx[k];
    if (idx->n == 0) {
        return false;
    }    
    hdr->depth[k] = 0;    
    hdr->path[k][0] = idx;
    hdr->pos[k][0] = 0;
    while (idx->tp == 1) {
        memcpy(&idx, idx->buf+idx->keylen, sizeof(memfh_idx_t *));
        int depth = hdr->depth[k];
        depth++;
        hdr->depth[k] = depth;
        hdr->path[k][depth] = idx;
        hdr->pos[k][depth] = 0;
    }
    return true;

}

bool memfh_idx_next(memfh_hdr_t *hdr, memfh_idx_t *idx, int k) {

    int depth = hdr->depth[k];
    if (hdr->pos[k][depth] < (idx->n - 1)) {
        hdr->pos[k][depth]++;
        return true;
    }    
    while (true) {
        if (depth == 0) {
            return false;
        }
        // avanca no pai
        depth--;
        hdr->depth[k] = depth;    
        memfh_idx_t *idx = hdr->path[k][depth];
        int pos = hdr->pos[k][depth];
        if (pos < idx->n) {
            pos++;    
            if (pos == idx->n) {
                idx = idx->next;

            } else {
                char *ptr = idx->buf + pos * (idx->keylen + sizeof(memfh_idx_t *));
                memcpy(&idx, ptr+idx->keylen, sizeof(memfh_idx_t *));
            }    
            hdr->pos[k][depth] = pos;

            // avanca ate chegar em uma folha
            while (idx->tp == 1) {
                // adiciona o no interno ao caminho de busca
                depth++;
                hdr->depth[k] = depth;    
                hdr->pos[k][depth] = 0;
                hdr->path[k][depth] = idx;
                memcpy(&idx, idx->buf+idx->keylen, sizeof(memfh_idx_t *));
            }

            // adiciona a folha ao caminho de busca
            depth++;
            hdr->depth[k] = depth;    
            hdr->pos[k][depth] = 0;
            hdr->path[k][depth] = idx;
            return true;

        } else {
            // TODO    
            return false;
        }    
    }
    return false;
}

void memfh_idx_create(memfh_hdr_t *hdr, int k) {

    char key0[257], key[257], *record;

    hdr->idx[k] = malloc(sizeof(memfh_idx_t));
    hdr->idx[k]->tp = 0;
    hdr->idx[k]->n = 0;
    hdr->idx[k]->next = NULL;

    if (!memfh_idx_first(hdr, 0)) {
        return;
    }

    // le o registro pela pk
    int depth = hdr->depth[k];
    memfh_idx_t *idx0 = hdr->path[k][depth];
    char *ptr0 = idx0->buf + hdr->pos[k][depth] * (idx0->keylen + sizeof(char *));
    memcpy(key0, ptr0, idx0->keylen);
    key0[idx0->keylen] = 0;    
    memcpy(&record, ptr0+idx0->keylen, sizeof(char *));

    // monta a chave
    int offset = 0;
    for (int c=0; c<hdr->keys[k][0]; c++) {
        memcpy(key, record + hdr->keys[k][1+c*2], hdr->keys[k][1+c*2+1]);
        offset += hdr->keys[k][1+c*2+1];
    }    
    key[offset] = 0;
    hdr->idx[k]->keylen = offset;

}

void memfh_idx_list(memfh_hdr_t *hdr) {

    char *ptr, key[257], *record;

    for (int k=0; k<hdr->nkeys; k++) {    
        fprintf(stderr, "=========================================== k=%d\n", k);    
        if (hdr->idx[k] == NULL) {
            continue;
        }    
        if (!memfh_idx_first(hdr, k)) {
            continue;
        }    
        int depth = hdr->depth[k];
        memfh_idx_t *idx = hdr->path[k][depth];
        ptr = idx->buf;
        memcpy(key, ptr, idx->keylen);
        key[idx->keylen] = 0;    
        memcpy(&record, ptr+idx->keylen, sizeof(char *));
        int r = 1;
        fprintf(stderr, "%09d %04d [%s] [%s]\n", r, hdr->pos[k][depth], key, record);
        while (memfh_idx_next(hdr, idx, k)) {
            depth = hdr->depth[k];
            idx = hdr->path[k][depth];
            ptr = idx->buf + hdr->pos[k][depth] * (idx->keylen + sizeof(char *));
            memcpy(key, ptr, idx->keylen);
            key[idx->keylen] = 0;    
            memcpy(&record, ptr+idx->keylen, sizeof(char *));
            fprintf(stderr, "%09d %04d [%s] [%s]\n", ++r, hdr->pos[k][depth], key, record);
        }
    }
}

void memfh_idx_show_page(memfh_hdr_t *hdr, memfh_idx_t *idx) {

    char *ptr, key[257], *record;

    fprintf(stderr, "idx n=%d %08x %08x\n", idx->n, (CAST) idx, (CAST) idx->next);
    ptr = idx->buf;
    for (int i=0; i<idx->n; i++) {
        memcpy(key, ptr, idx->keylen);
        key[idx->keylen] = 0;    
        if (idx->tp == 0) {
            memcpy(&record, ptr+idx->keylen, sizeof(char *));
            fprintf(stderr, "%d [%s] [%s]\n", i, key, record);
        } else {
            memfh_idx_t *page;
            memcpy(&page, ptr+idx->keylen, sizeof(memfh_idx_t *));
            fprintf(stderr, "%d [%s] %08x\n", i, key, (CAST) page);
        }        
        ptr += (idx->keylen + sizeof(char *));
    }        
}

bool memfh_idx_search_page(memfh_hdr_t *hdr, int k, memfh_idx_t *idx, char *key) {

    char *ptr = idx->buf;   
    memfh_idx_t *next;

    //fprintf(stderr, "memfh_idx_search_page %d [%s] %d %d\n", idx->keylen, key, idx->tp, idx->n);    

    int depth = hdr->depth[k];
    for (int i=0; i<idx->n; i++) {
        int c = memcmp(key, ptr, idx->keylen);
        if (c == 0) {
            hdr->pos[k][depth] = i;
            if (idx->tp == 0) {
                return true;
            } else {
                memcpy(&next, ptr+idx->keylen, sizeof(memfh_idx_t *));
                depth++;
                hdr->depth[k] = depth;
                hdr->path[k][depth] = next;
                return memfh_idx_search_page(hdr, k, next, key);
            }        
        }
        if (c < 0) {
            hdr->pos[k][depth] = i;
            if (idx->tp == 0) {
                return false;
            } else {
                memcpy(&next, ptr+idx->keylen, sizeof(memfh_idx_t *));
                depth++;
                hdr->depth[k] = depth;
                hdr->path[k][depth] = next;
                return memfh_idx_search_page(hdr, k, next, key);
            }
        }    
    }        

    hdr->pos[k][depth] = idx->n;
    if (idx->tp == 0) {
        return false;

    } else {    
        next = idx->next;
        depth++;
        hdr->depth[k] = depth;
        hdr->pos[k][depth] = 0;
        hdr->path[k][depth] = next;
        return memfh_idx_search_page(hdr, k, next, key);
    }    
}

bool memfh_idx_search(memfh_hdr_t *hdr, int k, char *key) {

    memfh_idx_t *idx = hdr->idx[k];
    hdr->path[k][0] = idx;
    hdr->pos[k][0] = 0;    
    hdr->depth[k] = 0;
    return memfh_idx_search_page(hdr, k, idx, key);

}

void memfh_idx_insert_parent(memfh_hdr_t *hdr, int k, int d, memfh_idx_t *idx1, memfh_idx_t *idx2) {

    memfh_idx_t *parent = hdr->path[k][d];

    if (parent->next == idx1) {
        // a pagina que quebrou era a ultima do pai    
        char *ptr = idx2->buf;
        // ptr = primeiro item da segunda pagina
        memcpy(parent->buf + parent->n * (parent->keylen + sizeof(memfh_idx_t *)), ptr, parent->keylen);
        // se for menor que o primeiro item da segunda pagina direciona para a primeira pagina
        memcpy(parent->buf + parent->n * (parent->keylen + sizeof(memfh_idx_t *)) + parent->keylen, &idx1, sizeof(memfh_idx_t *));
        parent->next = idx2;
        parent->n++;

        if (parent->n < MEMFH_MAX_ITEMS(parent->keylen)) {
            // tem espaco para inserir no pai    

        } else {
            memfh_idx_t *idx1 = parent;
            int n1 = idx1->n / 2;
            int n2 = idx1->n - n1;

            idx1->n = n1;
            memfh_idx_t *idx2 = malloc(sizeof(memfh_idx_t));
            idx2->keylen = idx1->keylen;
            idx2->tp = 1;
            idx2->next = idx1->next;
            idx1->next = idx2;
            idx2->n = n2;

            char *ptr1 = idx1->buf + n1 * (idx1->keylen + sizeof(char *));
            char *ptr2 = idx2->buf;

            for (int i=0; i<n2; i++) {
                memcpy(ptr2, ptr1, idx1->keylen + sizeof(char *));        
                ptr1 += (idx1->keylen + sizeof(char *));
                ptr2 += (idx1->keylen + sizeof(char *));
            }
            //memfh_idx_show_page(hdr, idx1, idx1->keylen);
            //memfh_idx_show_page(hdr, idx2, idx1->keylen);

            if (d == 0) {    
                parent = malloc(sizeof(memfh_idx_t));
                parent->keylen = idx1->keylen;
                parent->tp = 1;
                parent->n = 1;
                ptr2 = idx2->buf;
                memcpy(parent->buf, ptr2, idx1->keylen);
                memcpy(parent->buf + idx1->keylen, &idx1, sizeof(memfh_idx_t *));
                parent->next = idx2;

                hdr->idx[k] = parent;
                //memfh_idx_show_page(hdr, parent, idx1->keylen);

            } else {
                memfh_idx_insert_parent(hdr, k, d-1, idx1, idx2);
            }    
        }        

    } else {
        fprintf(stderr, "TODO a pagina que quebrou nao era a ultima do pai\n");    
        exit(-1);
    }
}

void memfh_idx_write(memfh_hdr_t *hdr, int k, char *key, char *record) {

    //fprintf(stderr, "memfh_idx_write %d [%s]\n", k, key);
    bool exists = memfh_idx_search(hdr, k, key);
    if (exists) {
        exit(-1);    
    }        

    int depth = hdr->depth[k];
    memfh_idx_t *idx = hdr->path[k][depth];

    if (hdr->pos[k][depth] < idx->n) {    
        fprintf(stderr, "TODO abrir espaco\n");
        exit(-1);
    }        

    char *ptr = idx->buf + hdr->pos[k][depth] * (idx->keylen + sizeof(char *));
    memcpy(ptr, key, idx->keylen);
    memcpy(ptr+idx->keylen, &record, sizeof(char *));
    idx->n++;

    if (idx->n < MEMFH_MAX_ITEMS(idx->keylen)) {
        //memfh_idx_show_page(hdr, idx, idx->keylen);

    } else {

        int n1 = idx->n / 2;
        int n2 = idx->n - n1;

        idx->n = n1;
        memfh_idx_t *idx2 = malloc(sizeof(memfh_idx_t));
        idx2->keylen = idx->keylen;
        idx2->tp = 0;
        idx2->next = NULL;
        idx2->n = n2;

        char *ptr1 = idx->buf + n1 * (idx->keylen + sizeof(char *));
        char *ptr2 = idx2->buf;

        for (int i=0; i<n2; i++) {
            memcpy(ptr2, ptr1, idx->keylen + sizeof(char *));        
            ptr1 += (idx->keylen + sizeof(char *));
            ptr2 += (idx->keylen + sizeof(char *));
        }
        //memfh_idx_show_page(hdr, idx, idx->keylen);
        //memfh_idx_show_page(hdr, idx2, idx->keylen);

        if (depth == 0) {    
            memfh_idx_t *parent = malloc(sizeof(memfh_idx_t));
            parent->keylen = idx->keylen;
            parent->tp = 1;
            parent->n = 1;
            ptr2 = idx2->buf;
            memcpy(parent->buf, ptr2, idx->keylen);
            memcpy(parent->buf + idx->keylen, &idx, sizeof(memfh_idx_t *));
            parent->next = idx2;

            hdr->idx[k] = parent;
            //memfh_idx_show_page(hdr, parent, idx->keylen);

        } else {
            memfh_idx_insert_parent(hdr, k, depth-1, idx, idx2);
        }    
    }        

}
