CC = gcc -Wall -Werror 
OBJS = json.o load.o

all: json

clean:
	rm -f json $(OBJS)

json: $(OBJS)
	$(CC) $(OBJS) ../list2.o -o json

.c.o: .c
	$(CC) -c $<
