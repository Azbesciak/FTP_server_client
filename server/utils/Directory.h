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
    void listDirFiles();
    static void removeDirectory(string directory);
    static void createDirectory(string directory);

    void listFiles();
private:
    static bool isDirectoryExist(string dirname);
    static void POSIXSlashes(string *windowsSlashes);
};


#endif //SERVER_DIRECTORY_H
