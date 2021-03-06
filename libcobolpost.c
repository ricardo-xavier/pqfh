
/* test program for libpq (postgresql) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <libpq-fe.h>

#ifdef PQFH
void pqfh_executa_sql(PGconn *conn, char *query, char *p_retorno, int *p_registros, char *p_linha);
void pqfh_sql_next(int *p_registro, char *p_linha);
#endif

extern int dbg;
extern int dbg_times;
extern int dbg_cmp;
extern FILE *flog;

extern long tempo_cobolpost, tempo_total;
extern int qtde_cobolpost, qtde_total;

static PGconn *conn=NULL;
PGresult *res;
int *qryhandle;

void do_exit(PGconn *conn) {
    if (dbg > 0) {
        fprintf(stderr, "%ld cobolpost do_exit\n", time(NULL));
    }
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
    struct timeval tv1, tv2;

    if ((dbg > 0) || (dbg_cmp > 2)) {
        fprintf(stderr, "%ld cobolpost executa_sql [%s]\n", time(NULL), query);
    }
#ifdef PQFH
    if (dbg_times > 1) {
        gettimeofday(&tv1, NULL);
    }
	pqfh_executa_sql(conn, query, p_retorno, p_registros, p_linha);
    if (dbg_times > 1) {
        gettimeofday(&tv2, NULL);
        long tempo = ((tv2.tv_sec * 1000000) + tv2.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        fprintf(flog, "%ld cobolpost executa_sql [%s] tempo=%ld\n", time(NULL), query, tempo);
        tempo_cobolpost += tempo;
        tempo_total += tempo;
        qtde_cobolpost++;
        qtde_total++;
    }
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
            s_linha = (char*)malloc( 4000 ); // o 10 � do tamanho da formatacao.. caso ele seja menor que 10 vai estourar     
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
                	s_linha_aux = (char*)malloc( strlen(s_linha_ret) + 100 ); // o 10 � do tamanho da formatacao.. caso ele seja menor que 10 vai estourar
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
    struct timeval tv1, tv2;

    if ((dbg > 0) || (dbg_cmp > 2)) {
        fprintf(stderr, "%ld cobolpost next %d\n", time(NULL), *p_registro);
    }
#ifdef PQFH
    if (dbg_times > 1) {
        gettimeofday(&tv1, NULL);
    }
	pqfh_sql_next(p_registro, p_linha);
    if (dbg_times > 1) {
        gettimeofday(&tv2, NULL);
        long tempo = ((tv2.tv_sec * 1000000) + tv2.tv_usec) - ((tv1.tv_sec * 1000000) + tv1.tv_usec);
        fprintf(flog, "%ld cobolpost next %d tempo=%ld\n", time(NULL), *p_registro, tempo);
        tempo_cobolpost += tempo;
        tempo_total += tempo;
        qtde_cobolpost++;
        qtde_total++;
    }
	return;
#endif
	    
	    i_campos = PQnfields(res);
            s_linha = (char*)malloc( 4000 ); // o 10 � do tamanho da formatacao.. caso ele seja menor que 10 vai estourar     
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
                s_linha_aux = (char*)malloc( strlen(s_linha_ret) + 100 ); // o 10 � do tamanho da formatacao.. caso ele seja menor que 10 vai estourar
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
	
    if (dbg == -1) {
        get_debug();
    }
    if (dbg > 0) {
        fprintf(stderr, "%ld cobolpost conecta [%s]\n", time(NULL), p_conexao);
    }
	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK)
	{
	   conninfo = "NAO CONECTADO";
        fprintf(stderr, "%ld Erro na conexao com o banco de dados: %s\n%s\n",
            time(NULL), PQerrorMessage(conn), conninfo);
        conn = NULL;
	}
	else
	{
	   conninfo = "CONECTADO";
        if (dbg > 0) {
            fprintf(stderr, "%ld cobolpost conectado\n", time(NULL));
        }
	};
        while (*conninfo) {
            *p_retorno++ = *conninfo++;
        }
}

void
sql_disconnect_db( int *dbhandle ) {
    if (dbg == -1) {
        get_debug();
    }
    if (dbg > 0) {
        fprintf(stderr, "%ld cobolpost disconnect\n", time(NULL));
    }
	PGconn *conn = (PGconn *)*dbhandle;
	PQfinish(conn);
}

void fechar_conexao() {
    if (dbg == -1) {
        get_debug();
    }
    if (dbg > 0) {
        fprintf(stderr, "%ld cobolpost fechar\n", time(NULL));
    }
    if (conn != NULL) {
	    PQfinish(conn);
        conn = NULL;
    }
}
