#ifndef JSON_H
#define JSON_H

typedef struct jobject_s {
    char name[257];
    char *value;
    struct jobject_s *children;
} jobject_t;

#endif
