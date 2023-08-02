#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "json.h"

#define ST_0 '0'

char addToken(list2_t *tokens, char tp, char *json, int *start, int i, int lin);

list2_t *lex(char *json, int dbg) {
    list2_t *tokens = NULL;
    char st = ST_0;
    int len = strlen(json);
    int lin = 1;
    int start = -1;
    token_t tok;

    for (int i = 0; i < len; i++) {
        char ch = json[i];
        if (isWhiteSpace(ch)) {
            if (ch == '\n') {
                lin++;
            }
            if (st != TOK_STRING && st != TOK_NUMBER) {
                continue;
            }
        }

        switch (st) {
            case ST_0:
                switch (ch) {
                    case '[':
                    case ']':
                    case '{':
                    case '}':
                    case ',':
                    case ':':
                        tok.tp = TOK_SYMBOL;
                        tok.buf = malloc(2);
                        tok.buf[0] = ch;
                        tok.buf[1] = 0;
                        tok.lin = lin;
                        tokens = list2_append(tokens, &tok, sizeof(token_t));
                        break;
                    case '"':
                        st = TOK_STRING;
                        start = i + 1;
                        break;
                    default:
                        if (isdigit(ch)) {
                            st = TOK_NUMBER;
                            start = i;
                            break;
                        }
                        if (strncmp(json+i, "true", 4)) {
                            tok.tp = TOK_BOOL;
                            tok.buf = strdup("true");
                            tok.lin = lin;
                            tokens = list2_append(tokens, &tok, sizeof(token_t));
                            i += 3;
                            break;
                        }
                        if (strncmp(json+i, "false", 5)) {
                            tok.tp = TOK_BOOL;
                            tok.buf = strdup("false");
                            tok.lin = lin;
                            tokens = list2_append(tokens, &tok, sizeof(token_t));
                            i += 4;
                            break;
                        }
                        fprintf(stderr, "ERR LEX lin=%d st=%c ch=%d %c\n", lin, st, ch, ch);
                        exit(-1);
                        break;
                }
                break;

            case TOK_STRING:
                if (ch == '"') {
                    if ((i > 0) && (json[i-1] == '\\')) {
                        break;
                    }
                    st = addToken(tokens, TOK_STRING, json, &start, i, lin);
                }
                break;

            case TOK_NUMBER:
                if (!isdigit(ch) && (ch != '.')) {
                    st = addToken(tokens, TOK_NUMBER, json, &start, i, lin);
                    i--;
                }
                break;
        }
    }

    return tokens;
}

char addToken(list2_t *tokens, char tp, char *json, int *start, int i, int lin) {
    token_t tok;
    int len = i - *start;

    tok.lin = lin;
    tok.tp = tp;
    tok.buf = malloc(len+1);
    strncpy(tok.buf, json + *start, len);
    tok.buf[len] = 0;
    tokens = list2_append(tokens, &tok, sizeof(token_t));
    *start = -1;
    return ST_0;
}
