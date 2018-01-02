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
    static const string ROOTDIR;
    void listDirFiles();
    static void removeDirectory(string name);
    static void createDirectory(string name);
    void listFiles();
private:
    static bool isDirectoryExist(string dirname);
    static string POSIXSlashes(string windowsSlashes);

};


#endif //SERVER_DIRECTORY_H
