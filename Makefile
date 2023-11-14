CC = gcc
TARGET = vite
OBJS = origin.c
$(TARGET) : origin.o
	$(CC) -o $(TARGET) $(OBJS) -lm
origin.o : origin.c
	$(CC) -c -o origin.o origin.c
clean :
	rm *.o vite

 
