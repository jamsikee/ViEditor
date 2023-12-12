CC = gcc
TARGET = vite
OBJS = origin.o
LIBS = -lm -lncurses

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

$(OBJS) : origin.c
	$(CC) -c -o $(OBJS) origin.c

clean :
	rm -f $(OBJS) $(TARGET)
