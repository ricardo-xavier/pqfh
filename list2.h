#ifndef LIST2_H
#define LIST2_H

typedef struct list2_s {
    void *buf;
    int size;
    struct list2_s *next;
    struct list2_s *prior;
} list2_t;

list2_t *list2_first(list2_t *list);
list2_t *list2_last(list2_t *list);
list2_t *list2_append(list2_t *list, void *buf, int size);
list2_t *list2_remove(list2_t *list, list2_t *item);
list2_t *list2_free(list2_t *list);
int list2_size(list2_t *list);
list2_t *list2_get(list2_t *list, int i);

#endif

