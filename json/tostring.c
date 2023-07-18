#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "json.h"

list2_t *appendArray(list2_t *lines, jelement_t element) {
    lines = list2_append(lines, "[", 2);
    for (list2_t *ptr = list2_first(element.children); ptr != NULL; ptr = ptr->next) {
        jelement_t *child = (jelement_t *) ptr->buf;
printf("======================== item %c\n", child->tp);
    }
    lines = list2_append(lines, "]", 2);
    return lines;
}

char *tostring(int dbg, jelement_t root) {
    list2_t *lines = NULL;
    list2_t *ptr;
    int len = 0;
    int ofs = 0;
    char *json;

    if (root.tp == TP_ARRAY) {
        lines = appendArray(lines, root);
    }
/*
    lines = list2_append(lines, root.tp == TP_ARRAY ? "[" : "{", 2);

    lines = list2_append(lines, root.tp == TP_ARRAY ? "]" : "}", 2);
*/

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
