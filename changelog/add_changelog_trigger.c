#include <libpq-fe.h>

extern PGresult *execcmd(char *cmd);
extern void execcmd_or_exit(char *cmd);
char *rtrim(char *s, int n);
void create_changelog_function(char *schema, char *tablename);

void add_changelog_trigger(char *_schema, char *_tablename) {
    char sql[257], tablename[257], schema[257], *p;
    strcpy(schema, rtrim(_schema, 256));
    strcpy(tablename, rtrim(_tablename, 256));
    if ((p = strchr(schema, ' ')) != NULL) *p = 0;
    if ((p = strchr(tablename, ' ')) != NULL) *p = 0;
    create_changelog_function(schema, tablename);
    sprintf(sql, "drop trigger if exists changelog_trigger_%s on %s.%s", tablename, schema, tablename);
    PGresult *res = execcmd(sql);
    PQclear(res);
    sprintf(sql, "create trigger changelog_trigger_%s after insert or update or delete on %s.%s for each row execute procedure public.changelog_function_%s()", tablename, schema, tablename, tablename);
    execcmd_or_exit(sql);
}
