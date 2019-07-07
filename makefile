CC = gcc -m32 -I/usr/include/postgresql -Wall -Werror
COB = /u/cobol/bin/cob
OBJS = pqfh.o list2.o meta.o fhtypes.o op_open.o op_close.o op_start.o kdb.o op_next_prev.o op_read_random.o pq2cob.o op_rewrite.o op_write.o op_delete.o create_table.o replicator.o utils.o command.o

all: pqfh2

clean:
	rm -f pqfh2 $(OBJS)

pqfh2: $(OBJS)
	$(COB) $(OBJS) -o pqfh -lpq -lpthread

.c.o: .c
	$(CC) -c $<
