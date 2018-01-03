//
// Created by jakub on 01.01.18.
//

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <pwd.h>
#include <cstring>
#include "Directory.h"
#include "ServerException.h"

void Directory::listDirFiles() {
    ;
}

void Directory::createDirectory(string directory) {
    POSIXSlashes(&directory);
    string path(directory);
    path = getRootDir() + path;
    if(mkdir(path.c_str(), 0777) < 0)
    {
        if(errno == EEXIST)
        {
            //TODO recursive mkdir
        }
        string errorMsg("500 ");
        errorMsg += strerror(errno);
        errorMsg += ".";
        throw new ServerException(errorMsg);
    }
}


void Directory::listFiles() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (getRootDir().c_str())) != NULL) {
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
    int pos = 0;
    if((pos= dirname.find(getRootDir())) < 0)
    {
        dirname = getRootDir() + dirname;
    }
    if(stat(dirname.c_str(), &st) == -1)
    {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

void Directory::removeDirectory(string directory) {
    POSIXSlashes(&directory);
    directory = getRootDir() + directory;
    if(!isDirectoryExist(directory))
    {
        throw new ServerException("550 Directory not exist!");
    }
    if(rmdir(directory.c_str()) == -1)
    {
        throw new ServerException("550 Directory not empty!");
    }
}

void Directory::POSIXSlashes(string *windowsSlashes) {
    int pos = 0;
    while((pos = windowsSlashes->find("\\") )>=0)
    {
        windowsSlashes->replace(pos, 1, "/");
    }
}

string Directory::getRootDir() {
    char *home = getenv("HOME");
    if(home == nullptr)
    {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    string serverHome(home);
    serverHome += "/ftp_server/";
    return serverHome;
}

