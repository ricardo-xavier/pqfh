#include "../pqfh.h"
#include <libpq-fe.h>
#include <string.h>

extern table_t *tab_open;
extern fcd_t *fcd_open;
extern PGconn *conn;
extern FILE *flog;
extern int dbg;

void apply_changelog() {
    PGresult *res, *res2;
    char     sql[257], id[33], action[33], new_status[33];
    fcd_t    fcd;
    list2_t  *ptr;
    column_t *col;
    int      c=0, offset=0, len, colno;
    char     aux[MAX_REC_LEN+1], *p1, *p2, *p;
    unsigned char opcode[2];

    if (tab_open == NULL) {
        return;
    }        
    if (dbg > 0) {
        fprintf(flog, "%s.%s %s\n", tab_open->schema, tab_open->name, fcd_open->file_name);
    }

    res = PQexec(conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(flog, "%s\n%s\n", "BEGIN", PQerrorMessage(conn));        
    }        
    PQclear(res);

    sprintf(sql, "declare cursor_columns cursor for select * from changelog where status='PENDING' and table_name='%s' order by id", tab_open->name);
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        return;
    }
    PQclear(res);
    memcpy(&fcd, fcd_open, sizeof(fcd_t));

    for (int i=0;;i++) {
        res = PQexec(conn, "fetch next in cursor_columns");
        if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
            PQclear(res);
            break;
        }
        strcpy(id, PQgetvalue(res, 0, 1));
        strcpy(action, PQgetvalue(res, 0, 4));
        if (dbg > 0) {
            fprintf(flog, "%d %s\n", i, action);
        }    
        colno = strcmp(action, "delete") ? 6 : 5;
        p1 = PQgetvalue(res, 0, colno);

        short reclen = getshort(fcd.rec_len);
        memset(fcd.record, 0, reclen);
        c = 0;
        offset = 0;

        for (ptr=tab_open->columns; ptr!=NULL; ptr=ptr->next) {
            col = (column_t *) ptr->buf;

            p2 = strchr(p1, '|');
            if (p2 != NULL) {
                *p2 = 0;
            }
            strcpy(aux, p1);
            len = strlen(aux);
            if (p2 != NULL) {
                p1 = p2 + 1;
            }
            //if (!strcmp(col->name, "sp0103desc")) {
                //sprintf(aux, "%-35s", "TESTE_CHANGELOG");
            //}    
            if (dbg > 1) {
                fprintf(flog, "    %d %d %s %c %d,%d [%s]\n", 
                    c, offset, col->name, col->tp, col->len, col->dec, aux);
            }

            switch (col->tp) {

                case 'n':
                    p = aux;
                    if (aux[0] == '-') {
                        p++;
                        len--;
                    }
                    if (col->dec == 0) {
                        memset(fcd.record + offset, '0', col->len - len);
                        memcpy(fcd.record + offset + (col->len - len), p, len);
                    } else {
                        if (len > 0) {    
                            memset(fcd.record + offset, '0', col->len - len + 1);
                            memcpy(fcd.record + offset + (col->len - len + 1), p, len - col->dec - 1);
                            memcpy(fcd.record + offset + (col->len - len + 1) + (len - col->dec - 1), p + len - col->dec, col->dec);
                        } else {
                            memset(fcd.record + offset, '0', col->len + col->dec);
                        }        
                    }
                    if (p != aux) {
                        fcd.record[offset + col->len - 1] |= 0x40;
                    }
                    break;

                default:
                    memcpy(fcd.record + offset, aux, len);
                    memset(fcd.record + offset + len, ' ', col->len - len);
                    break;
            }

            c++;
            offset += col->len;
        }

        if (!strcmp(action, "insert")) {
            putshort(opcode, OP_WRITE);
        } else if (!strcmp(action, "update")) {
            putshort(opcode, OP_REWRITE);
        } else {
            putshort(opcode, OP_DELETE);
        }        
        if (dbg > 1) {
            memcpy(aux, fcd.record, reclen);
            aux[reclen] = 0;
            fprintf(flog, "[%s]\n", aux);
        }
        EXTFH(opcode, &fcd);
        if (dbg > 0) {
            fprintf(flog, "status=%c%c %d\n", fcd.status[0], fcd.status[1], fcd.status[1]);
        }

        if (fcd.status[0] == '0') {
            strcpy(new_status, "SUCCESS");    
        } else {
            sprintf(new_status, "ERROR_%c_%d", fcd.status[0], fcd.status[1]);    
        }        
        sprintf(sql, "update changelog set status='%s' where status='PENDING' and table_name='%s' and id=%s", new_status, tab_open->name, id);
        res2 = PQexec(conn, sql);
        if (dbg > 0) {
            fprintf(flog, "%s\n", sql);    
        }
        if (PQresultStatus(res2) != PGRES_COMMAND_OK) {
            fprintf(flog, "%s\n%s\n", sql, PQerrorMessage(conn));        
        }        
        PQclear(res2);
    }

    res = PQexec(conn, "CLOSE cursor_columns");
    PQclear(res);

    res = PQexec(conn, "COMMIT");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(flog, "%s\n%s\n", "COMMIT", PQerrorMessage(conn));        
    }        
    PQclear(res);
}
