
/* test program for libpq (postgresql) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libpq-fe.h>

#ifdef PQFH
void pqfh_executa_sql(PGconn *conn, char *query, char *p_retorno, int *p_registros, char *p_linha);
void pqfh_sql_next(int *p_registro, char *p_linha);
#endif

PGconn *conn;
PGresult *res;
int *qryhandle;

void do_exit(PGconn *conn) {
	PQfinish(conn);
	exit(1);
};

/* this function assumes a command buffer of 80 characters */
void 
executa_sql(char *query, char *p_retorno, int *p_registros, char *p_linha) {
	
	char *s_query=query;
	char *s_retorno;
	char *s_linha;
	char *s_linha_aux;
        char *s_linha_ret;
        char *s_linha_fim;
	int  i_registros;
	int  i_campos;
	int  i;
        int  i_r;
        int  i_tamanho;	

#ifdef PQFH
	pqfh_executa_sql(conn, query, p_retorno, p_registros, p_linha);
	return;
#endif
	res = PQexec(conn, s_query);
        if (!res || PQresultStatus(res) != PGRES_TUPLES_OK)
	{
	    s_retorno = PQresultErrorMessage(res);
	    i_registros = 0;
	
	}
	else
	{
	    s_retorno = "EXECUTADO";
	    i_campos = PQnfields(res);
            i_registros = PQntuples(res);
	    *p_registros = i_registros;
            s_linha = (char*)malloc( 4000 ); // o 10 é do tamanho da formatacao.. caso ele seja menor que 10 vai estourar     
            if (i_registros != 0) 
            {
            	for (i = 0; i < i_campos; i++)
            	{
                	s_linha_ret = PQgetvalue(res,0,i);
                	i_tamanho = PQfsize(res,i);
                	if (i_tamanho == -1)
                	{
                   		i_tamanho = -100;
                	}
                	else
                	{
                   		i_tamanho = (i_tamanho * -1);
               		}
                	s_linha_aux = (char*)malloc( strlen(s_linha_ret) + 100 ); // o 10 é do tamanho da formatacao.. caso ele seja menor que 10 vai estourar
                	i_r = sprintf(s_linha_aux,"%s", s_linha_ret);
                	strcat(s_linha, s_linha_aux);
                	free(s_linha_aux);
            	}
            	while (*s_linha)
            	{
                	*p_linha++ = *s_linha++;
            	}
            }
	    while (*s_retorno)
            {
             	*p_retorno++ = *s_retorno++;
	    }
	}
}
void 
executa_sql_next(int *p_registro, char *p_linha) {
	
	char *s_retorno;
	char *s_linha;
	char *s_linha_aux;
        char *s_linha_ret;
        char *s_linha_fim;
	int  i_registros;
	int  i_campos;
	int  i;
        int  i_r;
        int  i_tamanho;	        	
#ifdef PQFH
	pqfh_sql_next(p_registro, p_linha);
	return;
#endif
	    
	    i_campos = PQnfields(res);
            s_linha = (char*)malloc( 4000 ); // o 10 é do tamanho da formatacao.. caso ele seja menor que 10 vai estourar     
            for (i = 0; i < i_campos; i++)
            {
                s_linha_ret = PQgetvalue(res,*p_registro-1,i);
                i_tamanho = PQfsize(res,i);
                if (i_tamanho == -1)
                {
                   i_tamanho = -100;
                }
                 else
                {
                   i_tamanho = (i_tamanho * -1);
                }
                s_linha_aux = (char*)malloc( strlen(s_linha_ret) + 100 ); // o 10 é do tamanho da formatacao.. caso ele seja menor que 10 vai estourar
                i_r = sprintf(s_linha_aux, "%s",  s_linha_ret);
                strcat(s_linha, s_linha_aux);
                free(s_linha_aux);
            
            }
            while (*s_linha)
            {
                *p_linha++ = *s_linha++;
            }
            *p_registro = *p_registro;
}
void
sql_status_message( int *dbhandle, char *message ) {
	PGconn *conn = (PGconn *)*dbhandle;
	char *s;
	s = PQerrorMessage(conn);
	while (*s) {
	}
}

void
sql_max_tuple( int *qryhandle, int *maxtuple ) {
	*maxtuple = PQntuples((PGresult *)*qryhandle);
}

void
sql_max_field( int *qryhandle, int *maxfield ) {
	*maxfield = PQnfields((PGresult *)*qryhandle);
}

void
sql_get_value( int *qryhandle, int *ntuple, int *nfield, char *value ) {
	char *s;
	s = PQgetvalue((PGresult *)*qryhandle,*ntuple,*nfield);
	while (*s) {
		if (*s == '\n') continue;
		*value++ = *s++;
	}
}

void
sql_get_tuple( int *qryhandle, int *ntuple, char *tuple ) {
	char *s;
	int nfield=0;
	int maxfield = PQnfields((PGresult *)*qryhandle);
	while (nfield < maxfield) {
		s = PQgetvalue((PGresult *)*qryhandle,*ntuple,nfield);
		while (*s) {
			*tuple++ = *s++;
		}
		nfield++;
	}
}

void
limpa_consulta() {
	
       PQclear(res);
}

void
conecta_db( char *p_conexao, char *p_retorno) {
	
	char *conninfo=p_conexao;
	
	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK)
	{
	   conninfo = "NAO CONECTADO";
	}
	else
	{
	   conninfo = "CONECTADO";
	};
        while (*conninfo) {
            *p_retorno++ = *conninfo++;
        }
}

void
sql_disconnect_db( int *dbhandle ) {
	PGconn *conn = (PGconn *)*dbhandle;
	PQfinish(conn);
}

void fechar_conexao() {
	PQfinish(conn);
}
