#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

void load(char *filename, char **buf);
int parse(jobject_t *obj, char *json, int i);

#define ST_START_OBJ   'O'
#define ST_START_NAME  'N'
#define ST_NAME        'n'
#define ST_COLON       ':'
#define ST_START_VALUE 'V'

char st;

int main(int argc, char *argv[]) {
    char *json;
    jobject_t root;

    if (argc != 2) {
        fprintf(stderr, "Syntax: %s <file.json>\n", argv[0]);
        return -1;
    }
    load(argv[1], &json);
    st = ST_START_OBJ;
    memset(&root, 0, sizeof(jobject_t));
    parse(&root, json, 0);
    free(json);
    return 0;
}

int parse(jobject_t *obj, char *json, int i) {
    printf("parse %s\n", obj->name);    
    jobject_t child;    
    for (;;) {    
        printf("i=%d %c st=%c\n", i, json[i], st);
        switch (st) {
            case ST_START_OBJ:
                switch (json[i]) {
                    case ' ':
                    case '\n':
                        break;    
                    case '[':
                        obj->isArray = true;    
                        break;
                    case '{':
                        memset(&child, 0, sizeof(jobject_t));
                        st = ST_START_NAME;
                        i = parse(&child, json, ++i);
                        obj->children = list2_append(obj->children, &child, sizeof(jobject_t));
                        break;    
                    default:
                        fprintf(stderr, "PARSE ERR %d '%c' st=%c\n", i, json[i], st);
                        exit(-1);
                        break;    
                }        
                break;

            case ST_START_NAME:
                switch (json[i]) {
                    case ' ':
                    case '\n':
                        break;    
                    case '"':
                        st = ST_NAME;
                        break;
                    default:
                        fprintf(stderr, "PARSE ERR %d '%c' st=%c\n", i, json[i], st);
                        exit(-1);
                        break;    
                }        
                break;

            case ST_NAME:
                if (json[i] != '"') {
                    int len = strlen(obj->name);
                    obj->name[len++] = json[i];
                    obj->name[len] = 0;
                } else {
                    st = ST_COLON;    
                }        
                break;        

            case ST_COLON:
                switch (json[i]) {
                    case ' ':
                    case '\n':
                        break;    
                    case ':':
                        st = ST_START_VALUE;
                        break;
                    default:
                        fprintf(stderr, "PARSE ERR %d '%c' st=%c\n", i, json[i], st);
                        exit(-1);
                        break;    
                }
                break;        

            default:
                fprintf(stderr, "PARSE ERR %d '%c' st=%c\n", i, json[i], st);
                exit(-1);
                break;    
        }
        i++;
    }    
    return i;
}
