CC = $(CXX)
INC_DIR = -Iutils
CFLAGS = -Wall -pthread $(INC_DIR)
CPP = server.cpp utils/directory.cpp
default: server
server: 	server.cpp
	$(CC) $(CFLAGS) $(CPP)  -o server
clean:		rm server.o
