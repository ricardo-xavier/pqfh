#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "pqfh.h"

extern int dbg;

void op_json(char *filename, unsigned short opcode, fcd_t *fcd) {
    char *p;
    if ((p = strchr(filename, ' ')) != NULL) {
        *p = 0;
    }
fprintf(stderr, "teste1\n");
    switch (opcode) {
        case OP_OPEN_INPUT:
            json_open(filename, fcd);
            break;
        case OP_START_EQ:
            json_start(filename, fcd);
            break;
        case OP_READ_NEXT:
            json_next(filename, fcd);
            break;
        case OP_CLOSE:
            json_close(filename, fcd);
            break;
    }
fprintf(stderr, "teste2\n");
short idx = getshort(fcd->json_ptr+4);
fprintf(stderr, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX %d\n", idx);
}
