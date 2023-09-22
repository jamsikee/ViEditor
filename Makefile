CC = gcc
TARGET = vite
OBJS = test.c
$(TARGET) : test.o
	$(CC) -o $(TARGET) $(OBJS) -lm
test.o : test.c
	$(CC) -c -o test.o test.c
clean :
	rm *.o vite

 
