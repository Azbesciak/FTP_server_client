//
// Created by jakub on 01.01.18.
//

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include "Directory.h"

const string Directory::ROOTDIR = "root/";

void Directory::listDirFiles() {

}

bool Directory::createDirectory(string name) {
    if(isDirectoryExist(name))
    {
        return false;
    }
    return mkdir((ROOTDIR + name.c_str()).c_str(), 0777) == 0 ? true : false;
}


void Directory::listFiles() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir ("root")) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if(ent->d_type != 4)
                printf ("%s, %d\n", ent->d_name, ent->d_type);
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("Directory error");
    }
}

bool Directory::isDirectoryExist(string dirname) {
    struct stat st = {0};

    if(stat(dirname.c_str(), &st) == -1)
    {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

void Directory::removeDirectory(string name) {
    if(!isDirectoryExist(name))
    {
        throw new string("550 Directory not exist!");
    }
    if(rmdir(name.c_str()) == -1)
    {
        throw new string("550 Directory not empty!");
    }
}

