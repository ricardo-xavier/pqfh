#include <libpq-fe.h>

extern PGresult *execcmd(char *cmd);
extern void execcmd_or_exit(char *cmd);
char *rtrim(char *s, int n);

void add_changelog_trigger(char *_schema, char *_tablename) {
    char sql[257], tablename[257], schema[257];

    strcpy(schema, rtrim(_schema, 256));
    strcpy(tablename, rtrim(_tablename, 256));
    sprintf(sql, "drop trigger if exists changelog_%s on %s.%s", tablename, schema, tablename);
    PGresult *res = execcmd(sql);
    PQclear(res);
    sprintf(sql, "create trigger changelog_%s after insert or update or delete on %s.%s for each row execute procedure public.changelog_trigger()", tablename, schema, tablename);
    execcmd_or_exit(sql);
}
