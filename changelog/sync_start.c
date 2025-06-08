#include "../pqfh.h"
#include <libpq-fe.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

list2_t *sync_tables;
list2_t *sync_fcds;

void sync_start() {
    sync_tables = NULL;    
    sync_fcds = NULL;    
}
