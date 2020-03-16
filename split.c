#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

void split(char *filename) {

    int MAXSZ = 1024 * 1024 * 1024;
    int fdin, fdout = -1, n, sz = MAXSZ, seq = 1;
    char buf[8193], splitname[257];
    unsigned short reclen = 0;

    if ((fdin = open(filename, O_RDONLY|O_LARGEFILE)) == -1) {
        fprintf(stderr, "Erro na abertura do arquivo [%s] #%d\n", filename, errno);    
        return;
    }

    n = read(fdin, buf, 128);
    if ((n != 128) || memcmp(buf, "0~", 2)) {
        fprintf(stderr, "Erro no formato do arquivo mf4\n");
        close(fdin);
        return;
    }

    while ((n = read(fdin, buf, 2)) == 2) {
        reclen = (unsigned char) buf[1];
        if ((sz + reclen) > MAXSZ) {
            if (fdout != -1) {
                close(fdout);
            }        
            sprintf(splitname, "%s_%02d", filename, seq++);
            fprintf(stderr, "%s\n", splitname);
            fdout = open(splitname, O_RDWR|O_CREAT|O_TRUNC, 0666);
            sz = 0;
        }
        read(fdin, buf, reclen);
        write(fdout, buf, reclen);
        write(fdout, "\n", 1);
        sz += reclen + 1;
    }

    close(fdin);    
    close(fdout);    
}    

int main(int argc, char *argv[]) {
    split(argv[1]);    
    return 0;        
}
