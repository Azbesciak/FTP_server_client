#pragma once
#include <vector>
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define FILTER_NOT_ALLOWED_FILES 4
using namespace std;

void createDirectory(std::string name);
int readFile(char *filename, char * buffer, long *bufferSize);
void listFiles();
void saveFile(char *filename);