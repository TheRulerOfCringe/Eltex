all: server client

server: server.c
	gcc -Wall -o server server.c
    
client:
	gcc -Wall -o client client.c

clean:
	rm -f server client
    
.PHONY: all clean
