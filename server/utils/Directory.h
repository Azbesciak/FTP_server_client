//
// Created by jakub on 01.01.18.
//

#ifndef SERVER_DIRECTORY_H
#define SERVER_DIRECTORY_H

#include <iostream>

using namespace std;

class Directory {

public:
    static string getRootDir();
    static void createDirectories(string directory, string currentDirectory);
    static void removeDirectory(string directory, string currentDirectory);
    static string listFiles(string directory, string currentDirectory);
    static string changeDirectory(string directory);

    static bool isFileExist(string filename);

private:
    static void createDirectory(string directory);
    static bool isDirectoryExist(string dirname);
    static bool isDescriptorExist(string descriptor, struct stat *st);

    static void slashesConverter(string *windowsSlashes);
    static unsigned int getSize(string fullname);
    static unsigned int getSize(string directory, string file);
    static void preparePath(string *path);
    static string convertRelativeAbsolutePath(string *directory, string *currentDirectory);
};


#endif //SERVER_DIRECTORY_H
