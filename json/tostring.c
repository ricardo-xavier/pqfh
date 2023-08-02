#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "json.h"

int level;
char buf[65537];

list2_t *appendArray(list2_t *lines, jelement_t element, char comma);

list2_t *appendObject(list2_t *lines, jelement_t element, char comma) {
    if (element.name != NULL) {
        sprintf(buf, "%s\"%s\": {", tab(level), element.name); 
    } else {
        sprintf(buf, "%s{", tab(level)); 
    }
    level++;
    lines = list2_append(lines, buf, strlen(buf)+1);
    for (list2_t *ptr = list2_first(element.children); ptr != NULL; ptr = ptr->next) {
        jelement_t *child = (jelement_t *) ptr->buf;
        switch (child->tp) {
            case TP_STRING:
                sprintf(buf, "%s\"%s\": \"%s\"%c", tab(level), child->name, child->value, ptr->next == NULL ? ' ' : ','); 
                lines = list2_append(lines, buf, strlen(buf)+1);
                break;
            case TP_NUMBER:
            case TP_BOOL:
                sprintf(buf, "%s\"%s\": %s%c", tab(level), child->name, child->value, ptr->next == NULL ? ' ' : ','); 
                lines = list2_append(lines, buf, strlen(buf)+1);
                break;
            case TP_ARRAY:
                lines = appendArray(lines, *child, ptr->next == NULL ? ' ' : ',');
                break;
            case TP_OBJECT:
                lines = appendObject(lines, *child, ptr->next == NULL ? ' ' : ',');
                break;
            default:
                fprintf(stderr, "ERR appendObject tp=%c\n", child->tp);
                exit(-1);
        }
    }
    level--;
    sprintf(buf, "%s}%c", tab(level), comma); 
    lines = list2_append(lines, buf, strlen(buf)+1);
    return lines;
}

list2_t *appendArray(list2_t *lines, jelement_t element, char comma) {
    if (element.name != NULL) {
        sprintf(buf, "%s\"%s\": [", tab(level), element.name); 
    } else {
        sprintf(buf, "%s[", tab(level)); 
    }
    level++;
    lines = list2_append(lines, buf, strlen(buf)+1);
    for (list2_t *ptr = list2_first(element.children); ptr != NULL; ptr = ptr->next) {
        jelement_t *child = (jelement_t *) ptr->buf;
        switch (child->tp) {
            case TP_OBJECT:
                level++;
                lines = appendObject(lines, *child, ptr->next == NULL ? ' ' : ',');
                level--;
                break;
            case TP_STRING:
                sprintf(buf, "%s\"%s\"%c", tab(level), child->value, ptr->next == NULL ? ' ' : ','); 
                lines = list2_append(lines, buf, strlen(buf)+1);
                break;
            case TP_NUMBER:
            case TP_BOOL:
                sprintf(buf, "%s%s%c", tab(level), child->value, ptr->next == NULL ? ' ' : ','); 
                lines = list2_append(lines, buf, strlen(buf)+1);
                break;
            default:
                fprintf(stderr, "ERR appendArray tp=%c\n", child->tp);
                exit(-1);
        }
    }
    level--;
    sprintf(buf, "%s]%c", tab(level), comma); 
    lines = list2_append(lines, buf, strlen(buf)+1);
    return lines;
}

char *tostring(int dbg, jelement_t root) {
    list2_t *lines = NULL;
    list2_t *ptr;
    int len = 0;
    int ofs = 0;
    char *json;

    level = 0;
    if (root.tp == TP_ARRAY) {
        lines = appendArray(lines, root, ' ');
    } else {
        lines = appendObject(lines, root, ' ');
    }

    for (ptr = list2_first(lines); ptr != NULL; ptr = ptr->next) {
        len += strlen(ptr->buf) + 2;
    }
    json = malloc(len+1);
    for (ptr = list2_first(lines); ptr != NULL; ptr = ptr->next) {
        memcpy(json+ofs, ptr->buf, strlen(ptr->buf));
        ofs += strlen(ptr->buf);
        memcpy(json+ofs, "\n", 1);
        ofs++;
    }
    json[ofs] = 0;
    return json;
}
