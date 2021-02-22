CC = /usr/local/arm-2014.05/bin/arm-none-linux-gnueabi-gcc 
#CC = gcc

APP_PATH = /home/weitao/nfs/usart
CFLAGS +=-I$(APP_PATH)
OBJ = test


all:
	$(CC) -o $(OBJ) $(CFLAGS) *.c -static -pthread
clean:
	rm -rf $(OBJ) *.o
gdb:
	$(CC) -g *.c -o $(OBJ)
link:
	$(CC) -c *.c
test:
	$(CC) -o $(OBJ) *.o
