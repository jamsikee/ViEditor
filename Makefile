CC = gcc

OS := $(shell uname -s)

ifeq ($(OS),Windows_NT)
    CFLAGS = -I pdcurses
    LDFLAGS = -L ./ -lpdcurses
else
    CFLAGS =
    LDFLAGS = -lncurses
endif

origin.o : origin.c
    $(CC) $(CFLAGS) -c origin.c -o origin.o $(LDFLAGS)
