#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list2.h"

list2_t *list2_first(list2_t *list) {
    if (list == NULL) {
        return NULL;
    }
    while (list->prior != NULL) {
        list = list->prior;
    }
    return list;
}

list2_t *list2_last(list2_t *list) {
    if (list == NULL) {
        return NULL;
    }
    while (list->next != NULL) {
        list = list->next;
    }
    return list;
}

list2_t *list2_append(list2_t *list, void *buf, int size) {
    list2_t *item;

    list = list2_last(list);

    item = (list2_t *) malloc(sizeof(list2_t));
    item->buf = malloc(size);
    memcpy(item->buf, buf, size);
    item->size = size;
    item->next = NULL;
    item->prior = NULL;

    if (list != NULL) {
        list->next = item;
        item->prior = list;
    }

    return item;
}

list2_t *list2_remove(list2_t *list, list2_t *item) {

    list2_t *ptr;

    if ((item->next == NULL) && (item->prior == NULL)) {
        free(item->buf);
        free(item);
        return NULL;
    }

    if (item->next == NULL) {
        item->prior->next = NULL;
        item->prior = NULL;
        list2_remove(NULL, item);
        return item->prior;
    }

    if (item->prior == NULL) {
        ptr = item->next;
        item->next->prior = NULL;
        item->next = NULL;
        list2_remove(NULL, item);
        return ptr;
    }

    ptr = item->next;
    item->next->prior = item->prior;
    item->prior->next = item->next;
    item->next = NULL;
    item->prior = NULL;
    list2_remove(NULL, item);
    return ptr;

}

list2_t *list2_free(list2_t *list) {
    list = list2_first(list);
    while (list != NULL) {
        list = list2_remove(list, list);
    }
    return NULL;
}

int list2_size(list2_t *list) {
    int sz = 0;
    list2_t *ptr = list2_first(list);
    while (ptr != NULL) {
        sz++;
        ptr = ptr->next;
    }
    return sz;
}

list2_t *list2_get(list2_t *list, int i) {
    int j = 0;
    list2_t *ptr = list2_first(list);
    while (ptr != NULL) {
        if (j++ == i) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}
