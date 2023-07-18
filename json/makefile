CC = gcc -g -Wall -Werror 
OBJS = json.o load.o lex.o parser.o tostring.o utils.o

all: json

clean:
	rm -f json $(OBJS)

json: $(OBJS)
	$(CC) $(OBJS) ../list2.o -o json

run:
	./json teste.json -dd >x 2>xx

.c.o: .c
	$(CC) -c $<
