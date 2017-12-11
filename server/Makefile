CC = $(CXX)
OUT_DIR = bin
INC_DIR = -Iutils
CFLAGS = -Wall -pthread $(INC_DIR)
CPP = server/server.cpp utils/directory.cpp
default:	SServer
SServer:	server/server.cpp
		$(CC) $(CFLAGS) $(CPP) -o $(OUT_DIR)/ftp_server
clean: 		rm ftp_server.o
