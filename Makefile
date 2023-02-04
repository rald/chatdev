CC=gcc
ARGS=-pthread -lncurses -ansi -std=c99 -O
CLEAN=rm -f

all: client server

client: client.c gui.c gui.h
	$(CC) $(ARGS) -o client client.c gui.c

server: server.c
	$(CC) $(ARGS) -o server server.c
	
clean:
	$(CLEAN) client server
