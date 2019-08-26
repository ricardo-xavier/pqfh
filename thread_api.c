#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <cobcall.h>
#include <jni.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "pqfh.h"

extern char *api;
extern int dbg;

void create_vm();

void *thread_api(void *vargp) {

    char metodo[9];

    api_args_t *args = (api_args_t *) vargp;
    if (dbg > 0) {
        fprintf(stderr, "thread_api %c [%s]\n", args->cmd, args->json);
    }

    switch (args->cmd) {
        case 'i':
            strcpy(metodo, "POST");
            break;
    }

    pqfh_call_java(api, args->api, metodo, args->json, "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIwNzg5MzMxMC05MmM3LTExZTktYTFmMC02OWM4ZjQzMjkxMDYiLCJhdWQiOiJBUEkiLCJmaWwiOjgsIm1hbCI6MSwiYW9mIjoxLCJ0aWQiOjIsImlhdCI6MTU2MDk3MTkxNCwiZXhwIjozMzA4NzAxNDMxNH0.3G_8Tbdh-USwV6z_BpzxBpMabbst61LGVNWvUEYkaus");

    return NULL;
}

void load_cpy(table_t *tab) {

    char cpy[257], buf[257], *p, *q;
    FILE *f;
    column_t col;

    sprintf(cpy, "%s.cpy", tab->api);
    if ((f = fopen(cpy, "r")) == NULL) {
        return;
    }

    fgets(buf, 257, f);
    if ((p = strpbrk(buf, "\r\n")) != NULL) *p = 0;
    strcpy(tab->api_root, buf+7);

    while (fgets(buf, 257, f) != NULL) {
        if ((p = strstr(buf, "03 ")) == NULL) {
            continue;
        }
        q = strchr(p, ' ');
        *q++ = 0;
        while (*q == ' ') q++;
        p = strchr(q, ' ');
        *p++ = 0;
        strcpy(col.name, q);
        q = p;
        p = strstr(q, "x(");
        if (p != NULL) {
            col.tp = 'x';
        } else {
            p = strstr(q, "9(");
            col.tp = '9';
        }
        p += 2;
        q = strchr(p, ')');
        *q = 0;
        col.len = atoi(p);
        tab->columns_api = list2_append(tab->columns_api, &col, sizeof(column_t));
    }

    fclose(f);
    tab->columns_api = list2_first(tab->columns_api);
}

void to_json(table_t *tab, char *record, char *json) {
    list2_t *ptr;
    column_t *col;
    char *p, *q, ch, aux[33];
    int offset=0;

    p = record;
    sprintf(json, "{ \"%s\" : [ {\n", tab->api_root);
    
    for (ptr=tab->columns_api; ptr!=NULL; ptr=ptr->next) {
        col = (column_t *) ptr->buf;
        strcat(json, "\t\"");
        strcat(json, col->name);
        strcat(json, "\" : ");
        p = record + offset;
        offset += col->len;
        q = p + col->len;
        ch = *q;
        *q = 0;
        if (col->tp == 'x') {
            strcat(json, "\"");
            strcat(json, p);
            strcat(json, "\"");
        } else {
            if (!*p) {
                strcat(json, "0");
            } else {
                sprintf(aux, "%d", atoi(p));
                strcat(json, aux);
            }
        }
        *q = ch;
        if (ptr->next != NULL) {
            strcat(json, ",");
        }
        strcat(json, "\n");
    }

    strcat(json, "} ] }");
}

int msgid=-1;
void call(int ppid);

void thread_api_start(char cmd, table_t *tab, fcd_t *fcd) {

    pthread_t thread_id;
    api_args_t args;
    char  json[MAX_REC_LEN+1];
    cobchar_t *argv[3];
    cobchar_t tabela[20];
    cobchar_t entrada[1000];
    cobchar_t saida[1000];
    argv[0] = tabela;
    argv[1] = entrada;
    argv[2] = saida;
    unsigned short reclen;

    reclen = getshort(fcd->rec_len);
    strcpy((char *) tabela, tab->name);
    memcpy((char *) entrada, fcd->record, reclen);
    entrada[reclen] = 0;
    cobcall((cobchar_t *) "converteapi", 3, argv);

    load_cpy(tab);
    to_json(tab, (char *) saida, json);

    strcpy(args.api, tab->api);
    args.cmd = cmd;
    strcpy(args.json, json);

    create_vm();
    pthread_create(&thread_id, NULL, thread_api, &args);
    sleep(1);
}
