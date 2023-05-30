CC = gcc
CFLAGS = -Wall

all: server client

server: server.o account.o check_login.o
	$(CC) $(CFLAGS) -o server server.o account.o check_login.o

client: client.o
	$(CC) $(CFLAGS) -o client client.o

server.o: server.c account.h check_login.h
	$(CC) $(CFLAGS) -c server.c

account.o: account.c account.h
	$(CC) $(CFLAGS) -c account.c

check_login.o: check_login.c check_login.h account.h
	$(CC) $(CFLAGS) -c check_login.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f server client *.o
