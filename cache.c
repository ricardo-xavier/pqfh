#include <string.h>
#include <malloc.h>
#include "pqfh.h"

extern int dbg;

list2_t *cache = NULL;

// procura uma tabela no cache e retorna seus metadados
table_t *cache_get(char *name) {
    funcao = _CACHE_GET;
    list2_t *ptr;
    table_t *table;
    for (ptr=cache; ptr!=NULL; ptr=ptr->prior) {
        table = (table_t *) ptr->buf;
        if (!strcmp(table->name, name)) {
            if (dbg > 2) {
                list2_t *ptr;
                for (ptr=table->keys; ptr!=NULL; ptr=ptr->next) {
                    _key_t *key = (_key_t *) ptr->buf;
                    for (int c=0; c<key->ncols; c++) {
                        list2_t *ptrcol;
                        int idx = 0;
                        for (ptrcol=table->columns; ptrcol!=NULL; ptrcol=ptrcol->next, idx++) {
                            if (idx == key->idxcolumns[c]) {    
                                key->columns[c] = (column_t *) ptrcol->buf;
                                break;
                            }    
                        }    
                    }        
                }             
                int offset = 0;
                int i = 0;
                for (ptr=table->columns; ptr!=NULL; ptr=ptr->next) {
                    column_t *col = (column_t *) ptr->buf;    
                    if (log_table(table->name)) fprintf(flog, "    %d %d %s %c %d\n", i, offset, col->name, col->tp, col->len);
                    offset += col->len;
                    i++;
                }    
                for (ptr=table->keys; ptr!=NULL; ptr=ptr->next) {
                    _key_t *key = (_key_t *) ptr->buf;
                    if (log_table(table->name)) {
                        fprintf(flog, "%ld key %d %d %d\n", time(NULL), key->id, key->ncomps, key->ncols);
                        for (int c=0; c<key->ncols; c++) {
                            fprintf(flog, "    %s\n", key->columns[c]->name);
                        }    
                    }
                }
            }
            return table;
        }
    }
    return NULL;
}

// adiciona os metadados de uma tabela ao cache
void cache_put(table_t *table) {
    funcao = _CACHE_PUT;
    list2_t *ptr;
    table_t *aux = (table_t *) malloc(sizeof(table_t));

    strcpy(aux->name, table->name);
    strcpy(aux->dictname, table->dictname);
    strcpy(aux->schema, table->schema);
    aux->convertida = table->convertida;
    aux->oid = table->oid;

    aux->columns = NULL;
    for (ptr=table->columns; ptr!=NULL; ptr=ptr->next) {
        aux->columns = list2_append(aux->columns, (column_t *) ptr->buf, sizeof(column_t));
    }
    aux->columns = list2_first(aux->columns);

    aux->keys = NULL;
    for (ptr=table->keys; ptr!=NULL; ptr=ptr->next) {
        _key_t *key = (_key_t *) ptr->buf;    
        for (int c=0; c<key->ncols; c++) {
            column_t *col1 = key->columns[c];
            list2_t *ptrcol;
            int idx = 0;
            for (ptrcol=table->columns; ptrcol!=NULL; ptrcol=ptrcol->next, idx++) {
                column_t *col2 = ptrcol->buf;    
                if (col2 == col1) {
                    key->idxcolumns[c] = idx;    
                    break;
                }
            }    
        }        
        aux->keys = list2_append(aux->keys, (_key_t *) ptr->buf, sizeof(_key_t));
    }
    aux->keys = list2_first(aux->keys);

    cache = list2_append(cache, aux, sizeof(table_t));
}

// remove uma tabela do cache
void cache_remove(char *name) {
    funcao = _CACHE_REMOVE;
    list2_t *ptr;
    table_t *table;
    for (ptr=cache; ptr!=NULL; ptr=ptr->prior) {
        table = (table_t *) ptr->buf;
        if (!strcmp(table->name, name)) {
            cache = list2_remove(ptr);    
            cache = list2_last(cache);
            break;
        }
    }
}
