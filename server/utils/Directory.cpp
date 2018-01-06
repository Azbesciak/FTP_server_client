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
#include "TerminalUtils.h"

void Directory::removeDirectory(string directory, string currentDirectory) {
    slashesConverter(&directory);
    string newPath = convertRelativeAbsolutePath(&directory, &currentDirectory);
    newPath += directory;

#if DEBUG
    cout << "Trying to remove " << directory << endl;
#endif

    if (!isDirectoryExist(newPath)) {
        throw ServerException("550 Folder nie istnieje.");
    }
    if (rmdir(newPath.c_str()) == -1) {
        throw ServerException("550 Folder nie jest pusty.");
    }
}

void Directory::createDirectories(string directory, string currentDirectory) {
    slashesConverter(&directory);
    string newPath = convertRelativeAbsolutePath(&directory, &currentDirectory);

    //to make while loop working properly
    if(directory[directory.size() - 1] != '/')
    {
        directory += '/';
    }

    size_t pos = 0;
    while ((pos = directory.find("/")) != string::npos) {
        string newDir(directory.substr(0, (int)pos));
        newPath += newDir;

#if DEBUG
        cout << "Trying to create " << newPath << endl;
#endif
        createDirectory(newPath);
        newPath += "/";
        directory.erase(0, (int)pos + 1);
    }
}

//używane tylko z createDirectories
void Directory::createDirectory(string directory) {
    slashesConverter(&directory);
    string path(directory);

    //add root path if doesn't included
    if (directory.find(getRootDir()) == string::npos) {
        path = getRootDir() + path;
    }
    if (mkdir(path.c_str(), 0777) < 0) {
        //if dir exists, do nothing
        if (errno != EEXIST) {
            string errorMsg("500 Nie udało się stworzyć folderu ");
            errorMsg += directory + ". ";
            errorMsg += strerror(errno);
            errorMsg += ".";
            throw ServerException(errorMsg);
        }
    }
}

/*
 * list /   -> listuje katalog głowny
 * list     -> listuje aktualny katalog
 * list dir -> listuje podkatalog dir
 */
string Directory::listFiles(string directory, string currentDirectory) {

    slashesConverter(&directory);

    if(directory == "/")
    {
        //listing folderu poczatkowego (root)
        directory = getRootDir() ;
    }
    else if(directory == currentDirectory)
    {
        //chcemy poznać zawartość aktualnego katalogu
        directory = getRootDir() + currentDirectory;
    } else {
        //chcemy poznac zawartosc podkatalogu w którym się znajdujemy

        //slash - usun na poczatku, dodaj na koncu
        preparePath(&directory);

        //add root prefix
        if ( directory.find(getRootDir()) == string::npos) {
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
    string value;
    if(!isDirectoryExist(directory))
    {
        throw ServerException("550 Folder nie istnieje!");
    }

#if DEBUG
    cout << "Trying to list " << directory << endl;
#endif

    if ((dir = opendir(directory.c_str())) != nullptr) {
        auto *size = new char[10];
        while ((ent = readdir(dir)) != nullptr) {
            //nie wypisuj folderów specjalnych
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
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
        throw ServerException("550 Nie można uzyskać dostępu do folderu.");
    }
    return value;
}


//return value contains / at the end
string Directory::changeDirectory(string directory) {
    slashesConverter(&directory);
    preparePath(&directory);

    if (directory == "/") {
        return directory;
    }


    string fullPath = getRootDir() + directory;

#if DEBUG
    cout << "Trying to change dir to " << fullPath << endl;
#endif

    if (!isDirectoryExist(fullPath)) {
        throw ServerException("550 Folder nie istnieje.");
    }
    return directory;
}


unsigned int Directory::getSize(string directory, string file)
{
    //dodaj slash na koniec nazwy folderu
    if (directory[directory.size() - 1] != '/') {
        directory += '/';
    }
    //usun slash na poczatku nazwy pliku
    if (file[0] == '/') {
        file.erase(0, 1);
    }
    return getSize(directory + file);
}

unsigned int Directory::getSize(string fullname)
{
    struct stat st = {0};
    if (stat(fullname.c_str(), &st) != -1) {
        return (unsigned int)st.st_size;
    }else{
        throw ServerException("550 Plik nie istnieje.");
    }
}

bool Directory::isDirectoryExist(string dirname) {
    struct stat st = {0};
    if(!isDescriptorExist(dirname, &st))
        return false;

    return S_ISDIR(st.st_mode);
}
bool Directory::isFileExist(string dirname) {
    struct stat st = {0};

    if(!isDescriptorExist(dirname, &st))
        return false;

    return S_ISREG(st.st_mode);
}

bool Directory::isDescriptorExist(string descriptor, struct stat *st) {
    //struct stat st = {0};
    if (descriptor.find(getRootDir()) == string::npos) {
        descriptor = getRootDir() + descriptor;
    }
    if (stat(descriptor.c_str(), st) == -1) {
        return false;
    }
    return true;
}

//converts backslashes to UNIX slashes
void Directory::slashesConverter(string *windowsSlashes) {
    size_t pos = 0;
    if (windowsSlashes->empty()) {
        return;
    }
    while ((pos = windowsSlashes->find("\\")) != string::npos) {
        windowsSlashes->replace((int)pos, 1, "/");
    }
}

string Directory::getRootDir() {
    //try to get home from env variable
    char *home = getenv("HOME");
    //try from passwd structure
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

//remove first slash and add at the end
//removes unnecessary slashes to avoid troublels with root dir
void Directory::preparePath(string *path)
{
    if ((*path)[0] == '/') {
        path->erase(0,1);   //remove first slash if exists
    }
    if ((*path)[path->size() - 1] != '/') {
        (*path) += '/'; //add slash at the end if doesn't exist
    }
}

//checks if path is relative or absolute and return proper string
string Directory::convertRelativeAbsolutePath(string *directory, string *currentDirectory) {
    string newPath(getRootDir());
    if ((*directory)[0] == '/') {
        //sciezka bezwzgledna lub katalog główny
        directory->erase(0, 1);
    } else {
        //sciezka wzgledna
        if ((*currentDirectory) != "/") {
            //dodaj pwd jezeli nie jest on /,
            // bo podfoldery nie zawieraja / na poczatku
            newPath += *currentDirectory;
        }
    }
    return newPath;
}

