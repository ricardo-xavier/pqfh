#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern bool dbg;
fcd_t fcd;
bool buildfcd=true;

list2_t *get_clones(char *tabname) {

    unsigned char opcode[2];
    char *envname, *p;
    clone_t clone;
    int pos;
    list2_t *clones=NULL;

    if (buildfcd) {
        buildfcd = false;
        envname = getenv("PQFH_REPLICA");
        if (envname == NULL) {
            fcd.file_name = (unsigned char *) strdup("../bd/replica");
        } else {
            fcd.file_name = (unsigned char *) strdup(envname);
        }

        fcd.organization = 2;    // indexed
        fcd.access_mode = 132;   // random + user file status
        fcd.open_mode = 128;     // closed
        putshort(fcd.file_name_len, strlen((char *) fcd.file_name));
        fcd.file_format = '4';

        fcd.record = malloc(156);
        putshort(fcd.rec_len, 155);

        fcd.kdb = malloc(40);
        memset(fcd.kdb, 0, 40);
        putshort(fcd.kdb+0, 40);
        putshort(fcd.kdb+6, 1);

        putshort(fcd.kdb+14+0, 1);
        putshort(fcd.kdb+14+2, 30);

        putint(fcd.kdb+30+2, 0);
        putint(fcd.kdb+30+6, 33);

        putshort(opcode, OP_OPEN_INPUT);
        EXTFH(opcode, &fcd);
        if (dbg > 2) {
            fprintf(stderr, "replica open %c%c %d\n", fcd.status[0], fcd.status[1], fcd.status[1]);
        }
    }

    memset(fcd.record, 0, 156);
    memcpy(fcd.record, tabname, strlen(tabname));

    putshort(opcode, OP_START_GE);
    EXTFH(opcode, &fcd);

    while (true) {
        putshort(opcode, OP_READ_NEXT);
        EXTFH(opcode, &fcd);

        if (memcmp(fcd.status, ST_OK, 2)) {
            break;
        }

        pos = MAX_NAME_LEN + 3;
        memcpy(clone.col1, fcd.record+pos, MAX_NAME_LEN);
        clone.col1[MAX_NAME_LEN] = 0;
        if ((p = strchr(clone.col1, ' ')) != NULL) *p = 0;

        pos += MAX_NAME_LEN;
        memcpy(clone.schema, fcd.record+pos, MAX_NAME_LEN);
        clone.schema[MAX_NAME_LEN] = 0;
        if ((p = strchr(clone.schema, ' ')) != NULL) *p = 0;

        pos += MAX_NAME_LEN;
        memcpy(clone.name, fcd.record+pos, MAX_NAME_LEN);
        clone.name[MAX_NAME_LEN] = 0;
        if ((p = strchr(clone.name, ' ')) != NULL) *p = 0;

        pos += MAX_NAME_LEN;
        memcpy(clone.col2, fcd.record+pos, MAX_NAME_LEN);
        clone.col2[MAX_NAME_LEN] = 0;
        if ((p = strchr(clone.col2, ' ')) != NULL) *p = 0;

        pos += MAX_NAME_LEN;
        clone.key = *(fcd.record+pos);

        pos++;
        clone.concat = *(fcd.record+pos);
        
        if (dbg > 2) {
            fprintf(stderr, "replica [%s] [%s] [%s] [%s] %c %c\n", clone.col1, clone.schema, clone.name, clone.col2, clone.key, clone.concat);
        }
        clones = list2_append(clones, &clone, sizeof(clone_t));
    }
    
    return list2_first(clones);
}

void prepare_write_clone(clone_t *clone, list2_t *clones) {
    list2_t *ptr;
    char  sql[4097], aux[9];
    int   seq=0;
    clone_t *c;

    sprintf(sql, "insert into %s.%s(", clone->schema, clone->name);

    for (ptr=clones; ptr!=NULL; ptr=ptr->next) {
        c = (clone_t *) ptr->buf;
        if (!strcmp(c->name, clone->name)) {
            if (seq > 0) {
                strcat(sql, ", ");
            }
            strcat(sql, c->col2);
            seq++;
        }
    }

    strcat(sql, ") values (");
    seq = 0;
    for (ptr=clones; ptr!=NULL; ptr=ptr->next) {
        c = (clone_t *) ptr->buf;
        if (!strcmp(c->name, clone->name)) {
            if (seq > 0) {
                strcat(sql, ", ");
            }
            sprintf(aux, "$%d", ++seq);
            strcat(sql, aux);
        }
    }
    strcat(sql, ")");

    fprintf(stderr, "%s\n", sql);
}

void replica_prepare_write(table_t *tab) {
    list2_t *ptr;
    clone_t *clone=NULL, *last=NULL;

    for (ptr=tab->clones; ptr!=NULL; ptr=ptr->next) {
        clone = (clone_t *) ptr->buf;
        if ((last != NULL) && strcmp(last->name, clone->name)) {
            prepare_write_clone(last, tab->clones);
        }
        last = clone;
    }

    if ((last != NULL))  {
        prepare_write_clone(last, tab->clones);
    }
}
