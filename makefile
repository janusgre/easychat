CC=gcc
CFLAGS=-I./include -lpthread

all : server client
.PHONY : all

client : ./src/client.c
	$(CC) -o client ./src/client.c $(CFLAGS)
server : ./src/server.c
	$(CC) -o server ./src/server.c $(CFLAGS) -lmysqlclient
clean :
	rm client
	rm server

