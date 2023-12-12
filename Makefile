CC = gcc
CFLAGS = -Wall -Wextra -g -I.
LDFLAGS = -L. -lpdcurses

TARGET = vite

SOURCES = origin.c

OBJECTS = $(SOURCES:.c=.o)

OS := $(shell uname -s)

ifeq ($(OS),Linux)
    LDFLAGS = -lncurses
else
    LDFLAGS = -lpdcurses
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
   $(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.c
   $(CC) $(CFLAGS) -c $<

clean:
   rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean