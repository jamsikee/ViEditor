CC = gcc
TARGET = vite
OBJS = origin.o

PDCURSES_DIR = ../vite/pdcurses

ifeq ($(OS),Windows_NT)
    PDCURSES_INC = -I$(PDCURSES_DIR)
    PDCURSES_DLL = $(PDCURSES_DIR)/pdcurses.dll
    LIBS = -lm $(PDCURSES_DLL)
    CFLAGS = $(PDCURSES_INC)

    $(TARGET): $(OBJS)
	    $(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
else
    LIBS = -lm -lncurses

    $(TARGET): $(OBJS)
	    $(CC) -o $(TARGET) $(OBJS) $(LIBS)
endif

$(OBJS) : origin.c
	$(CC) $(CFLAGS) -c origin.c

clean :
	rm -f $(OBJS) $(TARGET)
