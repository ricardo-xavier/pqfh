#ifndef JSON_H
#define JSON_H

#include "../list2.h"

#define bool unsigned char
#define false 0
#define true  1

#define TOK_SYMBOL '@'
#define TOK_STRING 'S'
#define TOK_NUMBER 'N'
#define TOK_BOOL   'B'

#define TP_ARRAY   'A'
#define TP_OBJECT  'O'
#define TP_STRING  'S'
#define TP_NUMBER  'N'
#define TP_BOOL    'B'

typedef struct token_s {
    int lin;
    char tp;
    char *buf;
} token_t;

typedef struct jelement_s {
    char *name;
    char tp;
    char *value;
    list2_t *children;
} jelement_t;

char *tab(int n);
bool isWhiteSpace(char ch);
bool load(char *filename, char **buf);
list2_t *lex(char *json, int dbg);
bool parse(char *json, int dbg, jelement_t *root);
char *tostring(int dbg, jelement_t root);

#endif
