CC = gcc
TARGET = vite
OBJS = origin.o

OS := $(shell uname -s)

ifeq ($(OS), Windows_NT)
    LIBS = -lm -lpdcurses
    INC = -I/path/to/pdcurses/header 
    LFLAGS = -L/path/to/pdcurses/lib 
else
    LIBS = -lm -lncurses
endif

$(TARGET) : $(OBJS)
    $(CC) $(LFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

$(OBJS) : origin.c
    $(CC) $(INC) -c -o $(OBJS) origin.c

clean :
    rm -f $(OBJS) $(TARGET)
