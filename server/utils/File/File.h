//
// Created by jakub on 01.01.18.
//

#ifndef SERVER_FILE_H
#define SERVER_FILE_H
#include <iostream>

using namespace std;

class File {
public:
    int readBinaryFile(string filename, char *buffer);
    void writeBinaryFile(string filename, char *buffer, int bufferSize);
    long readTextFile(string filename, char *buffer);
    void writeTextFile(string filename, char *buffer, int bufferSize);
    bool isFileExist(string filename);

private:
    long readFile(string filename, char * buffer, long *bufferSize, bool isBinary);
    void writeFile(string filename, char *buffer, int bufferSize, bool isBinary);

};


#endif //SERVER_FILE_H
