#include <string.h>
#include <stdlib.h>
#include "pqfh.h"

extern int dbg;
char schema[33];

fcd_t fcd01;
fcd_t fcd02;
bool montafcd01=true;
bool montafcd02=true;

bool tabela_convertida(char *tabela);
bool nome_dicionario(char *tabela, char *nome);

bool table_info(PGconn *conn, table_t *table, fcd_t *fcd) {

    PGresult   *res;
    char       sql[4097], aux[33];
    int        i, offset;
    column_t   col;
    unsigned short reclen;
    bool convertida;

    nome_dicionario(table->name, table->dictname);
    convertida = tabela_convertida(table->dictname);
    if (!convertida) {
        fcd->isam = 'S';
        return false;
    }

    table->columns = NULL;
    reclen = getshort(fcd->rec_len);

    // declara o cursor
    sprintf(sql, "declare cursor_columns cursor for  \nselect column_name,data_type,character_maximum_length,numeric_precision,numeric_scale\n    from information_schema.columns\n    where table_name = '%s'\n    order by ordinal_position", table->name);
    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
            PQerrorMessage(conn), sql);
        PQclear(res);
        return false;
    }
    PQclear(res);

    // le os registros do cursor
    offset = 0;
    for (i=0;;i++) {
        res = PQexec(conn, "fetch next in cursor_columns");
        if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
            break;
        }

        strcpy(col.name, PQgetvalue(res, 0, 0));
        col.offset = offset;

        strcpy(aux, PQgetvalue(res, 0, 1));

        if (!strcmp(aux, "numeric")) {
            col.tp = 'n';
            strcpy(aux, PQgetvalue(res, 0, 3));
            col.len = atoi(aux);
            strcpy(aux, PQgetvalue(res, 0, 4));
            col.dec = atoi(aux);

        } else if (!strcmp(aux, "character") || !strcmp(aux, "character varying")) {
            col.tp = 's';
            strcpy(aux, PQgetvalue(res, 0, 2));
            col.len = atoi(aux);
            col.dec = 0;

        } else {
            fprintf(stderr, "pendente type %s\n", aux);
            exit(-1);
        }

        if (dbg > 2) {
            fprintf(stderr, "    %d %d %s %c %d\n", i, offset, col.name, col.tp, col.len);
        }

        PQclear(res);
        if (offset < reclen) {
            col.pk = false;
            table->columns = list2_append(table->columns, &col, sizeof(column_t));
        }
        offset += col.len;

    }

    res = PQexec(conn, "CLOSE cursor_columns");
    PQclear(res);

    table->columns = list2_first(table->columns);
    
    getkeys(fcd, table);

    return table->columns != NULL ? true : false;
}

char *get_schema(PGconn *conn, char *table) {

    PGresult   *res;
    char       sql[4097];

    // declara o cursor
    sprintf(sql, "declare cursor_tables cursor for  \nselect table_schema\n    from information_schema.tables\n    where table_name = '%s'", table);
    if (dbg > 1) {
        fprintf(stderr, "%s\n", sql);
    }
    res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
            PQerrorMessage(conn), sql);
        PQclear(res);
        return "";
    }
    PQclear(res);

    // le o registro do cursor
    res = PQexec(conn, "fetch next in cursor_tables");
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)) {
        PQclear(res);
        res = PQexec(conn, "CLOSE cursor_tables");
        PQclear(res);
        return "";
    }

    strcpy(schema, PQgetvalue(res, 0, 0));

    res = PQexec(conn, "CLOSE cursor_tables");
    PQclear(res);
    return schema;
}

/*
 * fd pg01a01
 *    is external
 *    value of file-id is "../bd/pg01a01".
 *
 *    01  pg0101.
 *      03 pg0101key          pic x(26).
 *      03 filler redefines pg0101key.
 *        05 pg0101tabela     pic x(20).
 *        05 pg0101serie      pic 9(06).
 *      03 pg0101convertido   pic x. *> S ou N
 *      03 pg0101so-bd        pic x. *> S = atualizacao somente no banco de dados
 *      03 pg0101atu-ded      pic x. *> S = atualizacao pelo dedo duro, C = dedo-duro porem somente pelo cron
 *      03 pg0101atu-unit     pic x. *> S = atualizacao unitaria, nao utiliza bcpava somente o bcprdm
 *      03 pg0101filler       pic x(226).
 *
 *
 * select pg01a01 assign to disk
 *        file status is statuspg-a01
 *        organization is indexed
 *        access mode is dynamic
 *        record key is pg0101key
 *        alternate record key is pg0101key1 = pg0101serie pg0101tabela.
 *
 */

bool tabela_convertida(char *tabela) {

    unsigned char opcode[2];
    char *nomeenv;

    if (montafcd01) {
        montafcd01 = false;
        nomeenv = getenv("PQFH_PG01A01");
        if (nomeenv == NULL) {
            fcd01.file_name = (unsigned char *) strdup("../bd/pg01a01");
        } else {
            fcd01.file_name = (unsigned char *) strdup(nomeenv);
        }
        fcd01.organization = 2;    // indexed
        fcd01.access_mode = 132;   // random + user file status
        fcd01.open_mode = 128;     // closed
        putshort(fcd01.file_name_len, strlen((char *) fcd01.file_name));
        fcd01.file_format = '4';

        fcd01.record = malloc(257);
        putshort(fcd01.rec_len, 256);

        fcd01.kdb = malloc(76);
        memset(fcd01.kdb, 0, 76);
        putshort(fcd01.kdb+0, 76);
        putshort(fcd01.kdb+6, 2);

        putshort(fcd01.kdb+14+0, 1);
        putshort(fcd01.kdb+14+2, 46);

        putshort(fcd01.kdb+14+16+0, 2);
        putshort(fcd01.kdb+14+16+2, 56);

        putint(fcd01.kdb+46+2, 0);
        putint(fcd01.kdb+46+6, 26);

        putint(fcd01.kdb+56+2, 20);
        putint(fcd01.kdb+56+6, 6);

        putint(fcd01.kdb+66+2, 0);
        putint(fcd01.kdb+66+6, 20);

        putshort(opcode, OP_OPEN_INPUT);
        EXTFH(opcode, &fcd01);
    }

    memset(fcd01.record, 0, 256);
    memcpy(fcd01.record, tabela, strlen(tabela));

    putshort(opcode, OP_START_GE);
    EXTFH(opcode, &fcd01);

    putshort(opcode, OP_READ_NEXT);
    EXTFH(opcode, &fcd01);
    if (dbg > 2) {
        fprintf(stderr, "%s [%s] %c %c [%c]\n", fcd01.file_name, tabela, fcd01.status[0], fcd01.status[1], fcd01.record[26]);
    }

    return (fcd01.status[0] == '0') && (fcd01.status[1] == '0') && (fcd01.record[26] == 'S');
}

/*
 * fd  pg01a02
 *     is external
 *     value of file-id is "/u/rede/arqp/pg01a02".
 *
 * 01  pg0102.
 *   03 pg0102sigla      pic x(10).
 *   03 pg0102tabela     pic x(20).
 *   03 pg0102filler     pic x(226).
 *                                                 ~                                                                      
 * select pg01a02 assign to disk
 *        file status is statuspg-a02
 *        organization is indexed
 *        access mode is dynamic
 *        record key is pg0102sigla
 *        alternate record key is pg0102tabela.
 *
 */
bool nome_dicionario(char *tabela, char *nome) {

    unsigned char opcode[2];
    char *nomeenv, *p, prefixo[33];
    bool ret;

    strcpy(nome, tabela);
    if (montafcd02) {
        montafcd02 = false;
        nomeenv = getenv("PQFH_PG01A02");
        if (nomeenv == NULL) {
            fcd02.file_name = (unsigned char *) strdup("/u/rede/arqp/pg01a02");
        } else {
            fcd02.file_name = (unsigned char *) strdup(nomeenv);
        }
        fcd02.organization = 2;    // indexed
        fcd02.access_mode = 132;   // random + user file status
        fcd02.open_mode = 128;     // closed
        putshort(fcd02.file_name_len, strlen((char *) fcd02.file_name));
        fcd02.file_format = '4';

        fcd02.record = malloc(257);
        putshort(fcd02.rec_len, 256);

        fcd02.kdb = malloc(66);
        memset(fcd02.kdb, 0, 66);
        putshort(fcd02.kdb+0, 66);
        putshort(fcd02.kdb+6, 2);

        putshort(fcd02.kdb+14+0, 1);
        putshort(fcd02.kdb+14+2, 46);

        putshort(fcd02.kdb+14+16+0, 1);
        putshort(fcd02.kdb+14+16+2, 56);

        putint(fcd02.kdb+46+2, 0);
        putint(fcd02.kdb+46+6, 10);

        putint(fcd02.kdb+56+2, 10);
        putint(fcd02.kdb+56+6, 20);

        putshort(opcode, OP_OPEN_INPUT);
        EXTFH(opcode, &fcd02);
    }

    strcpy(prefixo, tabela);
    if (strlen(prefixo) > 4) {
        for (p=prefixo+4; *p; p++) {
            if (strchr("0123456789", *p) != NULL) {
                *p = 0;
                if (dbg > 2) {
                    fprintf(stderr, "prefixo [%s]\n", prefixo);
                }
                break;
            }
        }
    }

    memset(fcd02.record, 0, 256);
    memcpy(fcd02.record, prefixo, strlen(prefixo));
    putshort(fcd02.key_id, 0);

    putshort(opcode, OP_START_GE);
    EXTFH(opcode, &fcd02);

    putshort(opcode, OP_READ_NEXT);
    EXTFH(opcode, &fcd02);

    ret = (fcd02.status[0] == '0') && (fcd02.status[1] == '0') && !memcmp(fcd02.record, prefixo, strlen(prefixo));
    if (ret) {
        memcpy(nome, fcd02.record+10, 20);
        nome[20] = 0;
        if ((p = strchr(nome, ' ')) != NULL) *p=0;
    }
    if (dbg > 2) {
        fprintf(stderr, "%s dicionario [%s] %c %c [%s]\n", fcd02.file_name, tabela, fcd02.status[0], fcd02.status[1], nome);
    }
    return ret;
    
}
