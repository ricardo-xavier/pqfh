CC = gcc -m32 -I/usr/include/postgresql -Wall -Werror
COB = /u/cobol/bin/cob -m ixfile=pqfh
COB = /u/cobol/bin/cob
OBJS = pqfh.o list2.o meta.o fhtypes.o op_open.o op_close.o op_start_gt.o kdb.o op_read_next.o op_read_random.o pq2cob.o op_rewrite.o op_write.o op_delete.o

all: pqfh

clean:
	rm -f pqfh $(OBJS)

pqfh: $(OBJS)
	$(COB) $(OBJS) -o pqfh -lpq

.c.o: .c
	$(CC) -c $<
