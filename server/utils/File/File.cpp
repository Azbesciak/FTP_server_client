//
// Created by jakub on 01.01.18.
//

#include "File.h"

bool File::isFileExist(string filename) {
    FILE *fil = fopen(filename.c_str(), "r");
    if(fil == NULL)
    {
        return false;
    }
    return true;
}

void File::writeFile(string filename, char *buffer, int bufferSize, bool isBinary) {
    int counter;
    FILE *ptr_myfile;
    ptr_myfile=fopen(filename.c_str(), isBinary ? "wb" : "w");
    if (!ptr_myfile)
    {
        printf("Unable to open file!");
        return;
    }


    for ( counter=0; counter < 100; counter++)
    {
        printf("writing %d\n", counter);
        fwrite(&counter, sizeof(int), 1, ptr_myfile);
    }
    fclose(ptr_myfile);
}

long File::readFile(string filename, char *buffer, long *bufferSize, bool isBinary) {
    FILE *ptr_myfile;
    *bufferSize = 0;
    ptr_myfile=fopen(filename.c_str(), isBinary ? "rb" : "r");
    if(!ptr_myfile)
    {
        return 0;
    }

    //get flie size
    fseek(ptr_myfile, 0, SEEK_END);
    long filelen = ftell(ptr_myfile);
    rewind(ptr_myfile);


    buffer = (char *)malloc((filelen+1)*sizeof(char));
    fread(buffer, filelen, 1, ptr_myfile);
    *bufferSize = filelen;

    fclose(ptr_myfile);

    return *bufferSize;
}

long File::readTextFile(string filename, char *buffer) {
    long bufferSize = 0;
    readFile(filename, buffer, &bufferSize, false);
    return bufferSize;
}

void File::writeTextFile(string filename, char *buffer, int bufferSize) {
    writeFile(filename, buffer, bufferSize, false);
}

void File::writeBinaryFile(string filename, char *buffer, int bufferSize) {
    writeFile(filename, buffer, bufferSize, true);
}

int File::readBinaryFile(string filename, char *buffer) {
    long bufferSize = 0;
    readFile(filename, buffer, &bufferSize, true);
    return bufferSize;
}
