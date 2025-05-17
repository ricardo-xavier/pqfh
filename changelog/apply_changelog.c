#include "../pqfh.h"
#include <libpq-fe.h>
#include <string.h>

extern table_t *tab_open;
extern PGconn *conn;

void apply_changelog() {
    PGresult   *res;
    char       sql[257], action[33];

    printf("apply_changelog\n");     
    if (tab_open == NULL) {
        return;
    }        
    printf("%s\n", tab_open->name);

    sprintf(sql, "declare cursor_columns cursor for select * from changelog where status='PENDING' and table_name='%s' order by id", tab_open->name);
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        return; //TODO
    }
    PQclear(res);

    for (int i=0;;i++) {
        res = PQexec(conn, "fetch next in cursor_columns");
        if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
            PQclear(res);
            break;
        }
        strcpy(action, PQgetvalue(res, 0, 4));
        printf("%d %s\n", i, action);
    }    

    res = PQexec(conn, "CLOSE cursor_columns");
    PQclear(res);
}
    //char name[MAX_NAME_LEN+1];
    //char tp;
    //int len;
    //int dec;
    //int offset;
    //bool pk;
    //int p;
