#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pqfh.h"

list2_t *erros = NULL;

extern FILE *erropen();

void valida_comando(char *comando, char *tabela) {
    FILE *f;
    char user[257], *u;
    list2_t *ptr;

    if (erros == NULL) {
        return;
    }    

    if ((f = erropen()) == NULL) {
        return;
    }
    u = getenv("USER");
    if (u == NULL) {
        strcpy(user, "");
    } else {
        strcpy(user, u);
    }    
    fprintf(f, "%ld [%s]\nERRO DE VALIDACAO %s %s\n", time(NULL), user, comando, tabela);
    erros = list2_first(erros);
    for (ptr=erros; ptr!=NULL; ptr=ptr->next) {
        fprintf(f, "%s\n", (char *) ptr->buf);    
    }
    fprintf(f, "\n");
    fclose(f);
    erros = list2_free(erros);
}

bool valida_numero(char *coluna, char *conteudo, bool real) {

    char *p, aux[257];
    bool sinal=false, decimal=false;

    if (!conteudo[0]) {
        strcpy(conteudo, "0");
    }

    for (p=conteudo; *p; p++) {
        if (!isdigit(*p)) {
            if (!sinal && ((*p == '-') || (*p == '+'))) {
                sinal = true;    
                continue;
            }
            if (!decimal && (*p == '.') && real) {
                decimal = true;    
                continue;
            }
            sprintf(aux, "%s [%s]", coluna, conteudo);
            erros = list2_append(erros, aux, strlen(aux)+1);

            for (p=conteudo; *p; p++) {
                if (strchr("0123456789-.", *p) == NULL) {
                    *p = '0';
                }    
            }        
            return false;
        }
    }
    return true;
}
