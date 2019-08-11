#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "pqfh.h"

extern char *api;

void *thread_api(void *vargp) {

    char metodo[9];

    api_args_t *args = (api_args_t *) vargp;
    fprintf(stderr, "thread_api %c [%s]\n", args->cmd, args->json);

    switch (args->cmd) {
        case 'u':
            strcpy(metodo, "PUT");
            break;
    }

    pqfh_call_java(args->rota, metodo, args->json, "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIwNzg5MzMxMC05MmM3LTExZTktYTFmMC02OWM4ZjQzMjkxMDYiLCJhdWQiOiJBUEkiLCJmaWwiOjgsIm1hbCI6MSwiYW9mIjoxLCJ0aWQiOjIsImlhdCI6MTU2MDk3MTkxNCwiZXhwIjozMzA4NzAxNDMxNH0.3G_8Tbdh-USwV6z_BpzxBpMabbst61LGVNWvUEYkaus");

    args->tab->api_pending = false;
    return NULL;
}

void thread_api_start(char cmd, table_t *tab, fcd_t *fcd) {

    pthread_t thread_id;
    api_args_t args;
    list2_t *ptr, *ptrcol;
    char  json[MAX_REC_LEN+1];
    json_t  *j;
    column_t *col;
    char rota[257];

    strcpy(rota, api);
    strcat(rota, "/");
    strcat(rota, tab->api);

    strcpy(json, "{\n");

    for (ptr=list2_first(tab->json); ptr!=NULL; ptr=ptr->next) {
        j = (json_t *) ptr->buf;
        fprintf(stderr, "[%s] [%s]\n", j->colname, j->json);

        for (ptrcol=tab->columns; ptrcol!=NULL; ptrcol=ptrcol->next) {
            col = (column_t *) ptrcol->buf;
            if (!strcasecmp(col->name, j->colname)) {
                break;
            }
        }
        fprintf(stderr, "%d [%s]\n", col->p, col->name);

        strcat(json, "\"");
        strcat(json, j->json);
        strcat(json, "\" : ");
        if (col->tp != 'n') {
            strcat(json, "\"");
        }
        strcat(json, tab->bufs[col->p]);
        if (col->tp != 'n') {
            strcat(json, "\"");
        }
        if (ptr->next != NULL) {
            strcat(json, ",\n");
        }
    }

    strcat(json, "\n}");

    args.cmd = cmd;
    args.tab = tab;
    strcpy(args.rota, rota);
    strcpy(args.json, json);
    fprintf(stderr, "API START %c [%s]\n", cmd, tab->api);
    tab->api_pending = true;
    pthread_create(&thread_id, NULL, thread_api, &args);
}
