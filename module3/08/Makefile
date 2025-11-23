all: producer consumer

producer: producer.c
	gcc -o producer producer.c

consumer: consumer.c
	gcc -o consumer consumer.c

clean:
	rm -f producer consumer

.PHONY: all clean
