all: program

program: main.o contact.o
	gcc -Wall -o program main.o contact.o

main.o: main.c contact.h
	gcc -Wall -c main.c

contact.o: contact.c contact.h
	gcc -Wall -c contact.c

clean:
	rm -f program main.o contact.o
	
.PHONY: clean
