#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

bool load(char *filename, char **buf) {
    struct stat statbuf;
    FILE *f;
    int ofs, n, remaining;

    if (stat(filename, &statbuf) == -1) {
        fprintf(stderr, "ERR #%d stat %s\n", errno, filename);
        return false;
    }
    if ((f = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "ERR #%d fopen %s\n", errno, filename);
        return false;
    }
    remaining = (int) statbuf.st_size;
    *buf = (char *) malloc(remaining+1);
    ofs = 0;
    while (remaining > 0) {
        n = fread((*buf)+ofs, 1, remaining, f);
        ofs += n;
        remaining -= n;
    }
    (*buf)[ofs] = 0;
    fclose(f);
    return true;
}
