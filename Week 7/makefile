CC=gcc
CFLAGS=-Wall

all: client server

client: client.c
	$(CC) $(CFLAGS) -o client client.c -pthread

server: server.c
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f client server
