#include <stdlib.h>
#include <string.h>
#include "pqfh.h"

extern int dbg;
extern bool chaves_concatenadas;
extern int pending_commits;
extern char backup[MAX_REC_LEN+1];

bool op_rewrite(PGconn *conn, fcd_t *fcd) {

    unsigned int   fileid, keylen, keyoffset, remainder, offset;
    unsigned short reclen, keyid; 
    table_t        *tab;
    column_t       *pk, *col;
    char           record[MAX_REC_LEN+1], buf[257], sql[4097], aux[257], bufs[16][257];
    int            p, nParams;
    list2_t        *ptr;
    char           name[33];
    PGresult       *res;
    const char     *paramValues[256];
    int            paramFormats[256];
    int            paramLengths[256];

    fileid = getint(fcd->file_id);
    reclen = getshort(fcd->rec_len);

    tab = (table_t *) fileid;
    if (dbg > 0) {
        fprintf(stderr, "op_rewrite [%s]\n", tab->name);
    }

    keyid = getshort(fcd->key_id);
    putshort(fcd->key_id, 0);
    memcpy(record, fcd->record, reclen);

    // performance
    // verifica se o registro mudou antes de fazer o update
    op_read_random(conn, fcd);
    if (memcmp(fcd->status, ST_OK, 2)) {
        // registro nao encontrado
        return false;
    }
    if (!memcmp(record, fcd->record, reclen)) {
        memcpy(fcd->status, ST_OK, 2);
        if (dbg > 0) {
            fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
        }
        return false;
    }

    memcpy(backup, fcd->record, reclen);
    memcpy(fcd->record, record, reclen);

    kdb(fcd, &keyoffset, &keylen);
    pk = get_col_at(tab, keyoffset);
    if (pk == NULL) {
        fprintf(stderr, "coluna nao encontrada %d\n", keyoffset);
        exit(-1);
    }
    if (pk->len == keylen) {
        fprintf(stderr, "rewrite com chave simples nao implementado\n");
        exit(-1);
    }
    if (keyoffset != 0) {
        fprintf(stderr, "rewrite com keyoffset != 0 nao implementado\n");
        exit(-1);
    }

    memcpy(buf, fcd->record+keyoffset, keylen);
    buf[keylen] = 0;
    if (dbg > 1) {
        fprintf(stderr, "key %d %d:%d [%s]\n", 0, keyoffset, keylen, buf);
    }
    sprintf(name, "%s_upd", tab->name);

    // prepara o comando se ainda nao tiver preparado
    if (!tab->upd_prepared) {

        sprintf(sql, "update %s.%s\n", get_schema(conn, tab->name), tab->name);

        p = 0;
        remainder = keylen;
        for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

            col = (column_t *) ptr->buf;

            // ignora pk
            if (remainder > 0) {
                remainder -= col->len;
                continue;
            }

            if (p == 0) {
                strcat(sql, "  set ");
            } else {
                strcat(sql, "     ,");
            }
            tab->bufs[p] = malloc(col->len+2);
            sprintf(aux, "%s=$%d\n", col->name, ++p);
            strcat(sql, aux);
        }

        if (!chaves_concatenadas) {
            char where[4097];
            getwhere_prepared(tab, keyid, where, p, 'u');
            strcat(sql, "where ");
            strcat(sql, where);
            p += list2_size(tab->prms_rewrite);

        } else {
            // monta o where com as colunas da pk
            remainder = keylen;
            for (ptr=tab->columns; (remainder > 0) && (ptr != NULL); ptr=ptr->next) {
    
                col = (column_t *) ptr->buf;

                if (remainder == keylen) {
                    strcat(sql, "  where ");
                } else {
                    strcat(sql, "    and ");
                }
                tab->bufs[p] = malloc(col->len+2);
                sprintf(aux, "%s = $%d\n", col->name, ++p);
                strcat(sql, aux);

                remainder -= col->len;

            }
        }

        nParams = p;
        if (dbg > 1) {
            fprintf(stderr, "%s\n", sql);
        }
        tab->upd_prepared = true;

        res = PQprepare(conn, name, sql, nParams, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erro na execucao do comando: %s\n%s\n",
                PQerrorMessage(conn), sql);
            exit(-1);
        }
        PQclear(res);
    }

    // seta os parametros fora da pk
    p = 0;
    remainder = keylen;
    offset = 0;
    for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

        col = (column_t *) ptr->buf;

        // ignora pk
        if (remainder > 0) {
            remainder -= col->len;
            offset += col->len;
            continue;
        }

        paramValues[p] = tab->bufs[p];
        paramLengths[p] = col->len;
        paramFormats[p] = 0;

        if (col->tp == 'n') {
            if (col->dec > 0) {
                memcpy(tab->bufs[p], fcd->record + offset, col->len - col->dec);
                tab->bufs[p][col->len - col->dec] = '.';
                memcpy(tab->bufs[p] + col->len - col->dec + 1, fcd->record + offset + col->len - col->dec, col->dec);
                tab->bufs[p][col->len + 1] = 0;
            } else {
                memcpy(tab->bufs[p], fcd->record + offset, col->len);
                tab->bufs[p][col->len] = 0;
            }
            if ((fcd->record[offset + col->len - 1] & 0x40) == 0x40) {
                tab->bufs[p][0] = '-';
                if (col->dec > 0) {
                    tab->bufs[p][col->len] &= ~0x40;
                } else {
                    tab->bufs[p][col->len - 1] &= ~0x40;
                }
            }
        } else {
            memcpy(tab->bufs[p], fcd->record+offset, col->len);
            tab->bufs[p][col->len] = 0;
        }

        if (dbg > 2) {
            fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, offset, col->len, col->dec, tab->bufs[p]);
        }
        p++;
        offset += col->len;

    }

    if (!chaves_concatenadas) {
        int seq=0;
        for (ptr=tab->prms_rewrite; ptr!=NULL; ptr=ptr->next) {
            col = (column_t *) ptr->buf;
            paramValues[p] = bufs[seq];
            paramLengths[p] = col->len;
            paramFormats[p] = 0;

            if (dbg > 2) {
                fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, col->offset, col->len, col->dec, bufs[seq]);
            }
            seq++;
            p++;
        }

    } else {

        // seta os parametros da pk
        remainder = keylen;
        offset = 0;
        for (ptr=tab->columns; ptr!=NULL; ptr=ptr->next) {

            col = (column_t *) ptr->buf;

            // pk
            if (remainder > 0) {

                paramValues[p] = tab->bufs[p];
                paramLengths[p] = col->len;
                paramFormats[p] = 0;

                remainder -= col->len;
                memcpy(tab->bufs[p], fcd->record+offset, col->len);
                tab->bufs[p][col->len] = 0;

                if (dbg > 2) {
                    fprintf(stderr, "    %d %s %c %d:%d,%d [%s]\n", p, col->name, col->tp, offset, col->len, col->dec, tab->bufs[p]);
                }

                p++;
                offset += col->len;
            }

        }
    }
    nParams = p;

    // executa o comando
    res =  PQexecPrepared(conn, name, nParams, paramValues, paramLengths, paramFormats, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        memcpy(fcd->status, ST_REC_NOT_FOUND, 2);
        if (dbg > 0) {
            fprintf(stderr, "%s\n", PQerrorMessage(conn));
        }
    } else {
        memcpy(fcd->status, ST_OK, 2);
    }
    PQclear(res);
    pending_commits++;

    if (dbg > 0) {
        fprintf(stderr, "status=%c%c\n\n", fcd->status[0], fcd->status[1]);
    }
    putshort(fcd->key_id, keyid);
    return true;

}
