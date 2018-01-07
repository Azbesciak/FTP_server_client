CC = $(CXX)
OUT_DIR = bin
INC_DIR = -Iutils -Iserver
CPPFLAGS = -Wall -pthread $(INC_DIR) -std=c++11
CPP = server/Server.cpp utils/*.cpp
all:	Server
Server:		server/Server.cpp
		$(CC) $(CPPFLAGS) $(CPP) -o $(OUT_DIR)/ftp_server
clean: 		rm ftp_server.o
