#include <libpq-fe.h>

extern PGconn *conn;

extern PGresult *execcmd(char *cmd);
extern void execcmd_or_exit(char *cmd);

void create_changelog_function(char *schema, char *tablename) {
    char cmd[65537];    

    sprintf(cmd, "select column_name,data_type,character_maximum_length,numeric_precision,numeric_scale from information_schema.columns where table_schema = '%s' and table_name = '%s' order by ordinal_position", schema, tablename);
    PGresult *res = PQexec(conn, cmd);

    char name[257];
    char tp[257];
    char old[8193];
    char new[8193];
    char aux[257];
    int n = PQntuples(res);
    for (int i = 0; i < n; i++) {
        if (i == 0) {
            strcpy(old, "            old_data := ");
            strcpy(new, "            new_data := ");
        } else {
            strcat(old, " || '|' || ");
            strcat(new, " || '|' || ");
        }    
        strcpy(name, PQgetvalue(res, i, 0));
        strcpy(tp, PQgetvalue(res, i, 1));
        int dec = 0;
        if (!strcmp(tp, "character") || !strcmp(tp, "character varying")) {
            sprintf(aux, "coalesce(OLD.%s, '')", name);        
            strcat(old, aux);
            sprintf(aux, "coalesce(NEW.%s, '')", name);        
            strcat(new, aux);
        } else if (!strcmp(tp, "numeric")) {
            sprintf(aux, "coalesce(OLD.%s::varchar, '')", name);        
            strcat(old, aux);
            sprintf(aux, "coalesce(NEW.%s::varchar, '')", name);        
            strcat(new, aux);
        }                     
        strcat(old, "  ");        
        strcat(new, "  ");        
        
        if (i == (n - 1)) {
            strcat(old, ";");        
            strcat(new, ";");        
            break;
        }        
    }
    PQclear(res);

    res = PQexec(conn, "CLOSE cursor_columns");
    PQclear(res);    

    res = execcmd("CREATE LANGUAGE plpgsql");
    PQclear(res);
    sprintf(cmd, "create or replace function changelog_function_%s() returns trigger as $$ \n \
    declare \n \
        action text; \n \
        table_name text; \n \
        transaction_id bigint; \n \
        timestamp timestamp; \n \
        old_data text; \n \
        new_data text; \n \
    begin \n \
        action := lower(TG_OP::text); \n \
        table_name := TG_TABLE_NAME::text; \n \
        transaction_id := txid_current(); \n \
        timestamp := current_timestamp; \n \
        \n \
        if TG_OP = 'DELETE' then \n \
%s \n \
        elseif TG_OP = 'INSERT' then \n \
%s \n \
        elseif TG_OP = 'UPDATE' then \n \
%s \n \
%s \n \
        end if; \n \
        \n \
        insert into changelog \n \
        values ('PENDING', nextval('changelog_id_seq'::regclass), timestamp, table_name, action, old_data, new_data); \n \
        \n \
        return null; \n \
    end; \n \
    $$ language plpgsql;", tablename, old, new, old, new);
    //$$ language plpgsql;", tablename, "old_data := 'old';", "new_data := 'new';", "old_data := 'old';", "new_data := 'new';");
    //printf("%s\n", cmd);
    execcmd_or_exit(cmd);
}
