CC = gcc
ifeq ($(OS),Windows_NT)
	CFLAGS = -I pdcurses -L ./ -lpdcurses
else
	CFLAGS = -lm -lncurses
endif

origin.o : origin.c
	$(CC) -c -o origin.c $(CFLAGS)
