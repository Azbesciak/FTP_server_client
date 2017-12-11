INC_DIR = -Iutils
CFLAGS = -Wall -pthread $(INC_DIR)
default: server
server: 	server.c
		gcc $(CFLAGS) -o server server.c
clean:		rm server.o
