#ifndef MEMFH_H
#define MEMFH_H

#define MEMFH_PAGELEN    4096
#define MEMFH_MAX_KEYS   256
#define MEMFH_MAX_KEYLEN 256
#define MEMFH_MAX_DEPTH  16

#define MEMFH_LEAF   0
#define MEMFH_BRANCH 1

// -1 = espaco para adicionar o novo item antes de dividir a pagina
#define MEMFH_MAX_ITEMS(keylen) ((MEMFH_PAGELEN - 12) / (keylen + sizeof(char *)) - 1)

#ifndef bool
#define bool unsigned char
#define false 0
#define true 1
#endif

#define CAST int

typedef struct memfh_data_s {
    struct memfh_data_s *next;
    char *record;
} memfh_data_t;    

typedef struct memfh_idx_s {
    char tp;
    char filler;
    short keylen;
    int  n;    
    struct memfh_idx_s *next;
    char buf[MEMFH_PAGELEN-12];
} memfh_idx_t;    

typedef struct memfh_hdr_s {
    char *filename;    
    int   count;
    int   reclen;
    int   nkeys;
    int   **keys;
    memfh_data_t *head;
    memfh_data_t *tail;
    memfh_idx_t **idx;
    memfh_idx_t *path[MEMFH_MAX_KEYS][MEMFH_MAX_DEPTH];
    int pos[MEMFH_MAX_KEYS][MEMFH_MAX_DEPTH];
    int depth[MEMFH_MAX_KEYS];
} memfh_hdr_t;    

memfh_hdr_t *memfh_open(char *filename, int reclen, int nkeys, int **keys);
void memfh_close(memfh_hdr_t *hdr);
void memfh_write(memfh_hdr_t *hdr, char *record);
void memfh_rewrite(memfh_hdr_t *hdr, char *record);
bool memfh_read(memfh_hdr_t *hdr, char *record, bool update);
bool memfh_start(memfh_hdr_t *hdr, char *record, int k);
bool memfh_next(memfh_hdr_t *hdr, char *record, int k);
void memfh_list(memfh_hdr_t *hdr);

void memfh_idx_write(memfh_hdr_t *hdr, int k, char *key, char *record);
void memfh_idx_create(memfh_hdr_t *hdr, int k);
bool memfh_idx_first(memfh_hdr_t *hdr, int k);
bool memfh_idx_next(memfh_hdr_t *hdr, memfh_idx_t *idx, int k);
void memfh_idx_list(memfh_hdr_t *hdr);
void memfh_idx_list_k(memfh_hdr_t *hdr, int k);
void memfh_idx_show_page(memfh_hdr_t *hdr, memfh_idx_t *idx);
int memfh_idx_search(memfh_hdr_t *hdr, int k, char *key);
int memfh_idx_search_page(memfh_hdr_t *hdr, int k, memfh_idx_t *idx, char *key);

extern FILE *flog;

#endif // MEMFH_H

