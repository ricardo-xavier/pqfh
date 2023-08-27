#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

list2_t *parse_array(list2_t *ptr, int dbg, jelement_t *element);
list2_t *parse_object(list2_t *ptr, int dbg, jelement_t *element);
void err(char *msg, token_t *tok);

bool ok;

bool parse(char *json, int dbg, jelement_t *root) {
    list2_t *tokens;
    list2_t *ptr;

    tokens = lex(json, dbg);
    if (tokens == NULL) {
        return false;
    }
    tokens = list2_first(tokens);

    ok = true;
    if (dbg > 1) {
        for (ptr = tokens; ptr != NULL; ptr = ptr->next) {
            token_t *tok = (token_t *) ptr->buf;
            fprintf(stderr, "lin=%d tok=%c [%s]\n", tok->lin, tok->tp, tok->buf);
        }
    }

    token_t *tok = (token_t *) tokens->buf;
    if (!strcmp(tok->buf, "[")) {
        parse_array(tokens->next, dbg, root);
    } else if (!strcmp(tok->buf, "{")) {
        parse_object(tokens->next, dbg, root);
    } else {
        err("ERR expected [|{", tok);
    }
    return ok;
}

list2_t *parse_array(list2_t *ptr, int dbg, jelement_t *element) {
    jelement_t child;

    element->tp = TP_ARRAY;
    while (ptr != NULL) {
        token_t *tok = (token_t *) ptr->buf;
        switch (tok->buf[0]) {
            case ']':
                return ptr->next;
            case ',':
                ptr = ptr->next;
                break;
            case '{':
                if (dbg > 0) {
                    fprintf(stderr, "array object\n");
                }
                memset(&child, 0, sizeof(jelement_t));
                ptr = parse_object(ptr->next, dbg, &child);
                element->children = list2_append(element->children, &child, sizeof(jelement_t));
                break;
            default:
                if (dbg > 0) {
                    fprintf(stderr, "array %s\n", tok->buf);
                }
                memset(&child, 0, sizeof(jelement_t));
                child.tp = tok->tp;
                child.value = strdup(tok->buf);
                element->children = list2_append(element->children, &child, sizeof(jelement_t));
                ptr = ptr->next;
                break;
        } 
    }
    return ptr;
}

list2_t *parse_object(list2_t *ptr, int dbg, jelement_t *element) {
    jelement_t child;

    element->tp = TP_OBJECT;
    while (ptr != NULL) {
        token_t *tok = (token_t *) ptr->buf;
        switch (tok->buf[0]) {
            case '}':
                return ptr->next;
            case ',':
                ptr = ptr->next;
                break;
            default:
                if (tok->tp != TOK_STRING) {
                    err("ERR name expected", tok);
                }
                memset(&child, 0, sizeof(jelement_t));
                child.name = strdup(tok->buf);
                ptr = ptr->next;
                tok = (token_t *) ptr->buf;
                if (strcmp(tok->buf, ":")) {
                    err("ERR ':' expected", tok);
                }
                ptr = ptr->next;
                tok = (token_t *) ptr->buf;
                switch (tok->buf[0]) {
                    case '[':
                        if (dbg > 0) {
                            fprintf(stderr, "%s : %s\n", child.name, tok->buf);
                        }
                        ptr = parse_array(ptr->next, dbg, &child);
                        element->children = list2_append(element->children, &child, sizeof(jelement_t));
                        break;
                    case '{':
                        if (dbg > 0) {
                            fprintf(stderr, "%s : %s\n", child.name, tok->buf);
                        }
                        ptr = parse_object(ptr->next, dbg, &child);
                        element->children = list2_append(element->children, &child, sizeof(jelement_t));
                        break;
                    default:
                        if (dbg > 0) {
                            fprintf(stderr, "%s : %s\n", child.name, tok->buf);
                        }
                        child.tp = tok->tp;
                        child.value = strdup(tok->buf);
                        element->children = list2_append(element->children, &child, sizeof(jelement_t));
                        ptr = ptr->next;
                        break;
                }
                break;
        } 
    }
    return ptr;
}

void err(char *msg, token_t *tok) {
    fprintf(stderr, "%s lin=%d tp=%d buf=[%s]\n", msg, tok->lin, tok->tp, tok->buf);
    ok = false;
}
