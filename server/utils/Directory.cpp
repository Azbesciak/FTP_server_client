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
#include "Directory.h"
#include "ServerException.h"

void Directory::createDirectories(string directory, string currentDirectory) {
    POSIXSlashes(&directory);

    //remove first slash
    if (directory[0] == '/') {
        directory.erase(0, 1);
    }

    vector<string> splittedDirs;
    int pos = 0;
    string newPath(getRootDir());
    newPath += currentDirectory;

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
    unsigned long pos = 0;
    if ((pos = directory.find(getRootDir())) < 0) {
        path = getRootDir() + path;
    }
    if (mkdir(path.c_str(), 0777) < 0) {
        //if dir exists, do nothing
        if (errno != EEXIST) {
            string errorMsg("500 Nie udało się stworzyć folderu ");
            errorMsg += directory + ". ";
            errorMsg += strerror(errno);
            errorMsg += ".";
            throw new ServerException(errorMsg);
        }
    }
}


string Directory::listFiles(string directory,  string currentDirectory) {

    POSIXSlashes(&directory);

    if(directory == currentDirectory || directory == "/")
    {
        //chcemy poznać zawartość aktualnego katalogu
        directory = getRootDir() + currentDirectory + "/";
    } else {
        //remove slash at the beginning
        if (directory[0] == '/') {
            directory.erase(0, 1);
        }
        //add slash at the end
        if (directory[directory.size() - 1] != '/') {
            directory += '/';
        }

        int pos = 0;
        //add root prefix
        if ((pos = directory.find(getRootDir())) < 0) {
            if(currentDirectory == "/")
            {
                directory = getRootDir() + directory;
            }else
            {
                directory = getRootDir() + currentDirectory + directory;
            }
        }
    }


    DIR *dir;
    struct dirent *ent;
    string value = "";
    if(!isDirectoryExist(directory))
    {
        throw new ServerException("550 Folder nie istnieje!");
    }


    if ((dir = opendir(directory.c_str())) != nullptr) {
        char *size = new char[10];
        while ((ent = readdir(dir)) != nullptr) {
            //nie wypisuj folderów specjalnych
            if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            {
                continue;
            }

            switch (ent->d_type) {
                case
                    DT_REG:
                    value += 'P';   //plik
                    break;
                case
                    DT_DIR:
                    value += 'F';   //folder
                    break;
                default:
                    value += 'U';   //jakiś inny, np pipe
                    break;
            }
            value += ent->d_name;   //dodaj nazwe
            value += "/";
            memset(size, 0, 10);
            sprintf(size, "%d", getSize(directory, ent->d_name));
            value += size;
            value += char(3);
        }
        delete []size;
        closedir(dir);
    } else {
        throw new ServerException("550 Nie można uzyskać dostępu do folderu.");
    }
    return value;
}
unsigned int Directory::getSize(string directory, string file)
{
    //dodaj slash na koniec nazwy folderu
    if(directory[directory.size() - 1] != '/')
    {
        directory += '/';
    }
    //usun slash na poczatku nazwy pliku
    if(file[0] == '/')
    {
        file.erase(0, 1);
    }
    return getSize(directory + file);
}

unsigned int Directory::getSize(string fullname)
{
    struct stat st = {0};
    if(stat(fullname.c_str(), &st) != -1)
    {
        return (unsigned int)st.st_size;
    }else{
        throw new ServerException("550 Plik nie istnieje.");
    }
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

void Directory::removeDirectory(string directory, string currentDirectory) {
    POSIXSlashes(&directory);
    if(currentDirectory == "/")
    {
        directory = getRootDir() + directory;
    }
    else
    {
        directory = getRootDir() + currentDirectory + directory;
    }

    if (!isDirectoryExist(directory)) {
        throw new ServerException("550 Folder nie istnieje.");
    }
    if (rmdir(directory.c_str()) == -1) {
        throw new ServerException("550 Folder nie jest pusty.");
    }
}

void Directory::POSIXSlashes(string *windowsSlashes) {
    int pos = 0;
    if(windowsSlashes->size() == 0)
        return;
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

//return value contains / at the end
string Directory::changeDirectory(string directory) {
    POSIXSlashes(&directory);
    preparePath(&directory);

    //test
    //puste
    //slash
    //nieistniejace
    //istniejace

    if(directory == "/" || directory.empty())
    {
        return directory;
    }


    string fullPath = getRootDir() + directory;
    if(!isDirectoryExist(fullPath))
    {
        throw new ServerException("550 Folder nie istnieje.");
    }
    return directory;
}

void Directory::preparePath(string *path)
{
    if((*path)[0] == '/')
    {
        path->erase(0,1);   //remove first slash if exists
    }
    if((*path)[path->size() - 1] != '/')
    {
        (*path) += '/'; //add slash at the end if doesn't exist
    }
}

