//
// Created by jakub on 01.01.18.
//

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include "Directory.h"
#include "ServerException.h"

const string Directory::ROOTDIR = "/home/jakub/pp/FTP_server_client/root/";

void Directory::listDirFiles() {

}

void Directory::createDirectory(string name) {
    string path(name);
    path = ROOTDIR + path;
    mkdir(path.c_str(), 0777) ;
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
        throw new ServerException("550 Directory not exist!");
    }
    if(rmdir(name.c_str()) == -1)
    {
        throw new ServerException("550 Directory not empty!");
    }
}

string Directory::POSIXSlashes(string windowsSlashes) {
    int pos = 0;
    while((pos = windowsSlashes.find("\\") )>=0)
    {
        windowsSlashes.replace(pos, 1, "/");
    }
    return windowsSlashes;
}

