#include <string.h>
#include <stdlib.h>
#include "pqfh.h"

extern int dbg;

bool table_info(PGconn *conn, table_t *table, fcd_t *fcd) {

    PGresult   *res;
    char       sql[4097], aux[33];
    int        i, offset;
    column_t   col;
    unsigned short reclen;

    table->columns = NULL;
    reclen = getshort(fcd->rec_len);

    // declara o cursor
    sprintf(sql, "declare cursor_columns cursor for  \nselect column_name,data_type,character_maximum_length,numeric_precision,numeric_scale\n    from information_schema.columns\n    where table_name = '%s'\n    order by ordinal_position", table->name);
    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
            PQerrorMessage(conn), sql);
        PQclear(res);
        return false;
    }
    PQclear(res);

    // le os registros do cursor
    offset = 0;
    for (i=0;;i++) {
        res = PQexec(conn, "fetch next in cursor_columns");
        if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
            break;
        }

        strcpy(col.name, PQgetvalue(res, 0, 0));
        col.offset = offset;

        strcpy(aux, PQgetvalue(res, 0, 1));

        if (!strcmp(aux, "numeric")) {
            col.tp = 'n';
            strcpy(aux, PQgetvalue(res, 0, 3));
            col.len = atoi(aux);
            strcpy(aux, PQgetvalue(res, 0, 4));
            col.dec = atoi(aux);

        } else if (!strcmp(aux, "character") || !strcmp(aux, "character varying")) {
            col.tp = 's';
            strcpy(aux, PQgetvalue(res, 0, 2));
            col.len = atoi(aux);
            col.dec = 0;

        } else {
            fprintf(stderr, "pendente type %s\n", aux);
            exit(-1);
        }

        if (dbg > 2) {
            fprintf(stderr, "    %d %d %s %c %d\n", i, offset, col.name, col.tp, col.len);
        }

        PQclear(res);
        table->columns = list2_append(table->columns, &col, sizeof(column_t));
        offset += col.len;
        if (offset >= reclen) {
            break;
        }

    }

    res = PQexec(conn, "CLOSE cursor_columns");
    PQclear(res);

    table->columns = list2_first(table->columns);
    
    getkeys(fcd, table);

    return table->columns != NULL ? true : false;
}

char *get_schema(char *table) {
    if (strstr("sp01a03", table) != NULL) {
        return "basicos";
    }
    if (strstr("sp01a04", table) != NULL) {
        return "materiais";
    }
    if (strstr("cidades", table) != NULL) {
        return "basicos";
    }
    return "?";
}

column_t *get_col_at(table_t *table, unsigned int offset) {

    list2_t  *ptr;
    int      o=0;
    column_t *col;

    for (ptr=table->columns; ptr!=NULL; ptr=ptr->next) {
        col = (column_t *) ptr->buf;
        if (o == offset) {
            return col;
        }
        o += col->len;
    }
    return NULL;
}

