all: program sum min max

program: main.c
	gcc -Wall -o program main.c

sum: sum.c
	gcc -Wall -o sum sum.c
	
min: min.c
	gcc -Wall -o min min.c
	
max: max.c
	gcc -Wall -o max max.c

clean:
	rm -f program sum min max

.PHONY: clean
