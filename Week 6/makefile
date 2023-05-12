CC = gcc

CFLAGS = -Wall

all: client server

client: client.o
	$(CC) $(CFLAGS) -o client client.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

server: server.o
	$(CC) $(CFLAGS) -o server server.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f *.o client server
