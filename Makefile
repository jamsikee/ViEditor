CC = gcc
TARGET = vite
<<<<<<< HEAD
OBJS = origin.c
$(TARGET) : origin.o
	$(CC) -o $(TARGET) $(OBJS) -lm
origin.o : origin.c
	$(CC) -c -o origin.o origin.c
=======
OBJS = test.c
$(TARGET) : test.o
	$(CC) -o $(TARGET) $(OBJS) -lm
test.o : test.c
	$(CC) -c -o test.o test.c
>>>>>>> e5c253e42bd1974f8c8e469c65d77b81550d6271
clean :
	rm *.o vite

 
