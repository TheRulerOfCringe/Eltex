all: program1 program2

program1: program1.c
	gcc -o program1 program1.c
    
program2: program2.c
	gcc -o program2 program2.c

clean:
	rm -f program1 program2
	-mq_unlink /chat_queue1
	-mq_unlink /chat_queue2

.PHONY: all clean
