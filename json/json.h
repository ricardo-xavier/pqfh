#ifndef JSON_H
#define JSON_H

#include "../list2.h"

#define bool unsigned char
#define true 0

typedef struct jobject_s {
    char name[257];
    char *value;
    bool isArray;
    list2_t *children;
} jobject_t;

#endif
