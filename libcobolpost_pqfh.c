#include <stdio.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern int dbg_times;
PGresult *res;

void monta_linha(int linha, char *p_linha) {

    int num_campos, i, tipo_campo, tam_campo;
    char *valor_campo, buf_campo[257], fmt[9];

    num_campos = PQnfields(res);

    for (i=0; i<num_campos; i++) {
        valor_campo = PQgetvalue(res, linha, i);
        tipo_campo = PQftype(res, i);
        tam_campo = PQgetlength(res, linha, i);

        switch (tipo_campo) {
            case 1042: // character
            case 1043: // character varying
                sprintf(fmt, "%%-%ds", tam_campo);
                sprintf(buf_campo, fmt, valor_campo);
                break;

            default:
                sprintf(fmt, "%%0%dd%%s", 10-tam_campo);
                sprintf(buf_campo, fmt, 0, valor_campo);
                break;
        }
        strcat(p_linha, buf_campo);

        if (dbg > 1) {
            char *nome_campo = PQfname(res, i);

            fprintf(stderr, "    %d %s %d:%d [%s] [%s]\n", i, nome_campo, tipo_campo, 
                            tam_campo, valor_campo, buf_campo);
        }
    }

    if (dbg > 0) {
        fprintf(stderr, "%ld linha [%s]\n", time(NULL), p_linha);
    }    
}

void pqfh_executa_sql(PGconn *conn, char *query, char *p_retorno, int *p_registros, char *p_linha) {

    if (dbg == -1) {
        get_debug();
    }

    strcpy(p_linha, "");
    if (dbg > 0) {
        fprintf(stderr, "%ld executa_sql [%s]\n", time(NULL), query);
    }    

    res = PQexec(conn, query);

    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0))  {
        strcpy(p_retorno, PQresultErrorMessage(res));
        *p_registros = 0;

    } else {
        strcpy(p_retorno, "EXECUTADO");
        *p_registros = PQntuples(res);
        if (*p_registros > 0) {
            monta_linha(0, p_linha);
        }

    }

    PQclear(res);
}

void pqfh_sql_next(int *p_registro, char *p_linha) {
    if (dbg > 0) {
        fprintf(stderr, "%ld sql_next %d\n", time(NULL), *p_registro);
    }    
    monta_linha(*p_registro, p_linha);
}
