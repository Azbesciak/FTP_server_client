//
// Created by jakub on 01.01.18.
//

#ifndef SERVER_DIRECTORY_H
#define SERVER_DIRECTORY_H

#include <iostream>

#define FILTER_NOT_ALLOWED_FILES 4
using namespace std;

class Directory {

public:
    static string getRootDir();
    static void createDirectories(string directory);
    static void removeDirectory(string directory);
    static void createDirectory(string directory);
    static string listFiles(string directory);
private:
    static bool isDirectoryExist(string dirname);
    static void POSIXSlashes(string *windowsSlashes);
};


#endif //SERVER_DIRECTORY_H
