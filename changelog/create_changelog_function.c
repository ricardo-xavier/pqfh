#include <libpq-fe.h>

extern PGconn *conn;

extern PGresult *execcmd(char *cmd);
extern void execcmd_or_exit(char *cmd);

void create_changelog_function() {
    PGresult *res = execcmd("CREATE LANGUAGE plpgsql");
    PQclear(res);
    execcmd_or_exit("create or replace function changelog_trigger() returns trigger as $$ \n \
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
            old_data := 'to_jsonb(OLD.*)'; \n \
        elseif TG_OP = 'INSERT' then \n \
            new_data := 'to_json(NEW.*)'; \n \
        elseif TG_OP = 'UPDATE' then \n \
            old_data := 'to_jsonb(OLD.*)'; \n \
            new_data := 'to_jsonb(NEW.*)'; \n \
        end if; \n \
        \n \
        insert into changelog \n \
        values ('PENDING', nextval('changelog_id_seq'::regclass), timestamp, table_name, action, old_data, new_data); \n \
        \n \
        return null; \n \
    end; \n \
    $$ language plpgsql;");
}
