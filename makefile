CC = gcc -m32 -I/usr/include/postgresql -Wall -Werror
COB = /u/cobol/bin/cob
OBJS = pqfh.o list2.o meta.o fhtypes.o op_open.o op_close.o op_start.o kdb.o op_next_prev.o op_read_random.o pq2cob.o op_rewrite.o op_write.o op_delete.o create_table.o replicator.o utils.o command.o copy_table.o load_table.o cmp_table.o thread_api.o

all: pqfh

clean:
	rm -f pqfh $(OBJS)

pqfh: $(OBJS)
	$(COB) $(OBJS) -o pqfh -lpq -lpthread

.c.o: .c
	$(CC) -c $<
