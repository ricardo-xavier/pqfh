#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern bool dbg;
extern PGconn *conn2;
fcd_t fcd;
bool buildfcd=true;
bool replica_in_transaction;

list2_t *get_clones(char *tabname) {

    unsigned char opcode[2];
    char *envname, *p;
    clone_t clone;
    int pos;
    list2_t *clones=NULL;

    if (conn2 == NULL) {
        return NULL;
    }

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
            fprintf(flog, "%ld replica open %c%c %d\n", time(NULL), fcd.status[0], fcd.status[1], fcd.status[1]);
        }
        if (!memcmp(fcd.status, ST_OK, 2)) {
            fcd.open_mode = 0;
        }
    }

    if (fcd.open_mode == 128) {
        return NULL;
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
            fprintf(flog, "%ld replica [%s] [%s] [%s] [%s] %c %c\n", time(NULL), clone.col1, clone.schema, clone.name, clone.col2, clone.key, clone.concat);
        }
        clones = list2_append(clones, &clone, sizeof(clone_t));
    }
    
    return list2_first(clones);
}

void write_clone(table_t *tab, clone_t *clone, list2_t *clones) {
    list2_t *ptr, *ptrcol;
    char  sql[MAX_REC_LEN+1];
    int   seq=0;
    clone_t *c;
    column_t *col;
    PGresult *res;

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
            for (ptrcol=tab->columns; ptrcol!=NULL; ptrcol=ptrcol->next) {
                col = (column_t *) ptrcol->buf;
                if (!strcmp(col->name, c->col1)) {
                    if (col->tp != 'n') {
                        strcat(sql, "'");
                    }
                    strcat(sql, tab->bufs[col->p]);
                    if (col->tp != 'n') {
                        strcat(sql, "'");
                    }
                    break;
                }
            }
            seq++;
        }
    }
    strcat(sql, ")");

/*
    res = PQexec(conn2, "BEGIN");
    PQclear(res);
*/
    replica_in_transaction = true;

    if (dbg > 1) {
        fprintf(flog, "%ld %s\n", time(NULL), sql);
    }
    res = PQexec(conn2, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn2), sql);
        exit(-1);
    }
    PQclear(res);
}

void rewrite_clone(table_t *tab, clone_t *clone, list2_t *clones) {
    list2_t *ptr, *ptrcol;
    char  sql[MAX_REC_LEN+1];
    int   seq=0, seq2=0;
    clone_t *c;
    column_t *col;
    PGresult *res;

    sprintf(sql, "update %s.%s set ", clone->schema, clone->name);

    for (ptr=clones; ptr!=NULL; ptr=ptr->next) {
        c = (clone_t *) ptr->buf;
        if (!strcmp(c->name, clone->name) && (c->key == 'N')) {
            if (seq > 0) {
                strcat(sql, ", ");
            }
            strcat(sql, c->col2);
            strcat(sql, "=");
            for (ptrcol=tab->columns; ptrcol!=NULL; ptrcol=ptrcol->next) {
                col = (column_t *) ptrcol->buf;
                if (!strcmp(col->name, c->col1)) {
                    if (col->tp != 'n') {
                        strcat(sql, "'");
                    }
                    strcat(sql, tab->bufs[col->p]);
                    if (col->tp != 'n') {
                        strcat(sql, "'");
                    }
                    break;
                }
            }
            seq++;
        }
    }

    strcat(sql, " where ");

    for (ptr=clones; ptr!=NULL; ptr=ptr->next) {
        c = (clone_t *) ptr->buf;
        if (!strcmp(c->name, clone->name) && (c->key == 'S')) {
            if (seq2 > 0) {
                strcat(sql, " and ");
            }
            strcat(sql, c->col2);
            strcat(sql, "=");
            for (ptrcol=tab->columns; ptrcol!=NULL; ptrcol=ptrcol->next) {
                col = (column_t *) ptrcol->buf;
                if (!strcmp(col->name, c->col1)) {
                    if (col->tp != 'n') {
                        strcat(sql, "'");
                    }
                    strcat(sql, tab->bufs[col->p]);
                    if (col->tp != 'n') {
                        strcat(sql, "'");
                    }
                    break;
                }
            }
            seq2++;
        }
    }

/*
    res = PQexec(conn2, "BEGIN");
    PQclear(res);
*/
    replica_in_transaction = true;

    if (dbg > 1) {
        fprintf(flog, "%ld %s\n", time(NULL), sql);
    }
    res = PQexec(conn2, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn2), sql);
        exit(-1);
    }
    PQclear(res);
}

void delete_clone(table_t *tab, clone_t *clone, list2_t *clones) {
    list2_t *ptr, *ptrcol;
    char  sql[MAX_REC_LEN+1];
    int   seq=0;
    clone_t *c;
    column_t *col;
    PGresult *res;

    sprintf(sql, "delete from %s.%s where ", clone->schema, clone->name);

    for (ptr=clones; ptr!=NULL; ptr=ptr->next) {
        c = (clone_t *) ptr->buf;
        if (!strcmp(c->name, clone->name) && (c->key == 'S')) {
            if (seq > 0) {
                strcat(sql, " and ");
            }
            strcat(sql, c->col2);
            strcat(sql, "=");
            for (ptrcol=tab->columns; ptrcol!=NULL; ptrcol=ptrcol->next) {
                col = (column_t *) ptrcol->buf;
                if (!strcmp(col->name, c->col1)) {
                    if (col->tp != 'n') {
                        strcat(sql, "'");
                    }
                    strcat(sql, tab->bufs[col->p]);
                    if (col->tp != 'n') {
                        strcat(sql, "'");
                    }
                    break;
                }
            }
            seq++;
        }
    }

/*
    res = PQexec(conn2, "BEGIN");
    PQclear(res);
*/
    replica_in_transaction = true;

    if (dbg > 1) {
        fprintf(flog, "%ld %s\n", time(NULL), sql);
    }
    res = PQexec(conn2, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(flog, "%ld Erro na execucao do comando: %s\n%s\n", time(NULL), PQerrorMessage(conn2), sql);
        exit(-1);
    }
    PQclear(res);
}

void replica_write(table_t *tab) {
    list2_t *ptr;
    clone_t *clone=NULL, *last=NULL;

    for (ptr=tab->clones; ptr!=NULL; ptr=ptr->next) {
        clone = (clone_t *) ptr->buf;
        if ((last != NULL) && strcmp(last->name, clone->name)) {
            write_clone(tab, last, tab->clones);
        }
        last = clone;
    }

    if ((last != NULL))  {
        write_clone(tab, last, tab->clones);
    }
}

void replica_rewrite(table_t *tab) {
    list2_t *ptr;
    clone_t *clone=NULL, *last=NULL;

    for (ptr=tab->clones; ptr!=NULL; ptr=ptr->next) {
        clone = (clone_t *) ptr->buf;
        if ((last != NULL) && strcmp(last->name, clone->name)) {
            rewrite_clone(tab, last, tab->clones);
        }
        last = clone;
    }

    if ((last != NULL))  {
        rewrite_clone(tab, last, tab->clones);
    }
}

void replica_delete(table_t *tab) {
    list2_t *ptr;
    clone_t *clone=NULL, *last=NULL;

    for (ptr=tab->clones; ptr!=NULL; ptr=ptr->next) {
        clone = (clone_t *) ptr->buf;
        if ((last != NULL) && strcmp(last->name, clone->name)) {
            delete_clone(tab, last, tab->clones);
        }
        last = clone;
    }

    if ((last != NULL))  {
        delete_clone(tab, last, tab->clones);
    }
}
void replica_commit() {
/*
    PGresult *res;
    res = PQexec(conn2, "COMMIT");
    PQclear(res);
    replica_in_transaction = false;
*/
    if (dbg > 1) {
        fprintf(flog, "%ld replica commit", time(NULL));
    }
}

void replica_rollback() {
/*
    PGresult *res;
    res = PQexec(conn2, "ROLLBACK");
    PQclear(res);
    replica_in_transaction = false;
*/
    if (dbg > 1) {
        fprintf(flog, "%ld replica rollback", time(NULL));
    }
}
