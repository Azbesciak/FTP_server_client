//
// Created by jakub on 01.01.18.
//

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <pwd.h>
#include <cstring>
#include <iterator>
#include <vector>
#include <sstream>
#include "Directory.h"
#include "ServerException.h"

void Directory::createDirectories(string directory) {
    POSIXSlashes(&directory);

    //remove first slash
    if (directory[0] == '/') {
        directory.erase(0, 1);
    }

    vector<string> splittedDirs;
    int pos = 0;
    string newPath(getRootDir());
    while ((pos = directory.find("/")) >= 0) {
        string newDir(directory.substr(0, pos));
        newPath += newDir;
        createDirectory(newPath);
        newPath += "/";
        directory.erase(0, pos + 1);
    }

    if (pos == -1) {
        //no / in directory == no parent directories
        createDirectory(newPath + directory);
    }
}

void Directory::createDirectory(string directory) {
    POSIXSlashes(&directory);
    string path(directory);

    //add root path if doesn't included
    int pos = 0;
    if ((pos = directory.find(getRootDir())) < 0) {
        path = getRootDir() + path;
    }
    if (mkdir(path.c_str(), 0777) < 0) {
        //if dir exists, do nothing
        if (errno != EEXIST) {
            string errorMsg("500 Cannot create directory ");
            errorMsg += directory + ". ";
            errorMsg += strerror(errno);
            errorMsg += ".";
            throw new ServerException(errorMsg);
        }
    }
}


string Directory::listFiles(string directory) {
    int pos = 0;
    if (directory == "/") {
        //just slash as an argument
        if (directory == "/") {
            directory = getRootDir();
        }
    } else {
        //remove slash at the beginning
        if (directory[0] == '/') {
            directory.erase(0, 1);
        }
        //add slash at the end
        if (directory[directory.size() - 1] != '/') {
            directory += '/';
        }

        //add root prefix
        if ((pos = directory.find(getRootDir())) < 0) {
            directory = getRootDir() + directory;
        }
    }


    DIR *dir;
    struct dirent *ent;
    string value = "";
    if(!isDirectoryExist(directory))
    {
        throw new ServerException("550 Directory doesn't exist!");
    }

    if ((dir = opendir(directory.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            {
                continue;
            }

            switch (ent->d_type) {
                case
                    DT_REG:
                    value += 'P';
                    break;
                case
                    DT_DIR:
                    value += 'F';
                    break;
                default:
                    value += 'U';
                    break;
            }
            value += ent->d_name;
            value += char(3);
        }
        closedir(dir);
    } else {
        throw new ServerException("550 Cannot open directory.");
    }
    value.erase(value.size() - 1, 1);
    return value;
}

bool Directory::isDirectoryExist(string dirname) {
    struct stat st = {0};
    int pos = 0;
    if ((pos = dirname.find(getRootDir())) < 0) {
        dirname = getRootDir() + dirname;
    }
    if (stat(dirname.c_str(), &st) == -1) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

void Directory::removeDirectory(string directory) {
    POSIXSlashes(&directory);
    directory = getRootDir() + directory;
    if (!isDirectoryExist(directory)) {
        throw new ServerException("550 Directory not exist!");
    }
    if (rmdir(directory.c_str()) == -1) {
        throw new ServerException("550 Directory not empty!");
    }
}

void Directory::POSIXSlashes(string *windowsSlashes) {
    int pos = 0;
    while ((pos = windowsSlashes->find("\\")) >= 0) {
        windowsSlashes->replace(pos, 1, "/");
    }
}

string Directory::getRootDir() {
    char *home = getenv("HOME");
    if (home == nullptr) {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    string serverHome(home);
    serverHome += "/ftp_server/";

    //create ftp_server in home dir if not exist
    struct stat st = {0};
    if (stat(serverHome.c_str(), &st) == -1) {
        mkdir(serverHome.c_str(), 0777);
    }

    return serverHome;
}

