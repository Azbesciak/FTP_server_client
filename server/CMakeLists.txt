cmake_minimum_required(VERSION 3.9)
project(server)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-pthread -Wtraditional")
include_directories(.)
include_directories(server)
include_directories(utils)

add_executable(server
        server/Server.cpp
        server/Server.h
        Makefile utils/Client.cpp utils/Client.h utils/Directory.cpp utils/Directory.h utils/File/File.cpp utils/File/File.h utils/FTP.cpp utils/FTP.h utils/ServerException.h utils/TerminalUtils.h server/ServerConfig.h)
