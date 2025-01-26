#include <libpq-fe.h>

extern PGconn *conn;

extern void execcmd_or_exit(char *cmd);

void create_changelog_table() {
    if (conn == NULL) {
        dbconnect();
    }

    char *sql = "select table_name from information_schema.tables where table_name = 'changelog'";
    PGresult *res = PQexec(conn, sql);
    if ((PQresultStatus(res) == PGRES_TUPLES_OK) && (PQntuples(res) == 1)) {
        PQclear(res);
        return;    
    }        
    PQclear(res);

    execcmd_or_exit("create table changelog( \n \
	    status char(10) not null, \n \
	    id serial not null, \n \
	    timestamp timestamp not null, \n \
	    table_name varchar(40), \n \
	    action varchar(10), \n \
	    old_data text, \n \
	    new_data text, \n \
	    primary key(status, id) \n \
	    );");
}

