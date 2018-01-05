//
// Created by jakub on 01.01.18.
//

#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <iterator>
#include <sstream>

#include "FTP.h"
#include "Directory.h"
#include "ServerException.h"
#include "TerminalUtils.h"

string FTP::toUpper(string data) {
    std::transform(data.begin(), data.end(), data.begin(), ::toupper);
    return data;
}

void FTP::parseCommand(char *command) {
    string data(command);
    this->parseCommand(data);
}

void FTP::parseCommand(string command) {
    if (command.size() > 30) {
        throw ServerException("500 Komenda za długa.");
    }

    vector<string> splittedCommand = splitCommand(command);

    if (splittedCommand.empty()) {
        throw ServerException("500 Błąd w składni.");
    }

    splittedCommand[0] = toUpper(splittedCommand[0]);

    if (splittedCommand[0].find("TYPE") != string::npos) {
        if (splittedCommand.size() < 2) {
            throw ServerException("501 Błąd w składni parametrów.");
        }
        setTransferType(splittedCommand[1]);
    } else if (splittedCommand[0].find("MKDIR") != string::npos) {
        if (splittedCommand.size() < 2) {
            throw ServerException("501 Błąd w składni parametrów.");
        }
        string dirToCreate = getDirectoryWithSpaces(splittedCommand);
        makeDirectory(dirToCreate);
    } else if (splittedCommand[0].find("RMDIR") != string::npos) {
        if (splittedCommand.size() < 2) {
            throw ServerException("501 Błąd w składni parametrów.");
        }
        string directoryToRemove = getDirectoryWithSpaces(splittedCommand);
        removeDirectory(directoryToRemove);
    } else if (splittedCommand[0].find("LIST") != string::npos) {
        if (splittedCommand.size() < 2) {
            listFiles(currentDirectory);
        } else {
            listFiles(splittedCommand[1]);
        }
    } else if(splittedCommand[0].find("PWD") != string::npos)  {
        //wypisz zawartrosc zmiennej currentDirectory
        printDirectory();
    } else if(splittedCommand[0].find("CWD") != string::npos)
    {
        if(splittedCommand.size() < 2)
        {
            changeDirectory("/");   //brak parametru, przejdz do glownego
        } else
        {
            changeDirectory(splittedCommand[1]);    //przejdz do wskazanego przez parametr
        }
    } else if(splittedCommand[0].find("PORT") != string::npos)
    {
        throw ServerException("500 Niezaimplementowana komenda.");
    }else if(splittedCommand[0].find("PASSV") != string::npos)
    {
        throw ServerException("500 Niezaimplementowana komenda.");
    }else if(splittedCommand[0].find("RETR") != string::npos)
    {
        //pobieranie plików z serwera do klienta
        throw ServerException("500 Niezaimplementowana komenda.");
    }else if(splittedCommand[0].find("STOR") != string::npos)
    {
        //wysylanie plikow od klineta na serwer
        throw ServerException("500 Niezaimplementowana komenda.");
    } else {
        throw ServerException("500 Komenda nierozpoznana.");
    }
}

FTP::FTP(int socket) {
    this->socket = socket;
    currentDirectory = "/";
}

void FTP::sendResponse(string message) {
    message += "\r\n";
    cout << "\t" << MAGENTA_TEXT("odpowiedź do " << socket << ":\t") << GREEN_TEXT(message);
    write(socket, message.c_str(), message.size());
}

//rozbija komende na zbior wyrazen, co spacje
vector<string> FTP::splitCommand(string command) {
    vector<string> pieces;
    istringstream iss(command);
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter(pieces));
    return pieces;
}

//directory methods
void FTP::removeDirectory(string name) {
    Directory::removeDirectory(name, currentDirectory);
    sendResponse("200 OK");
}
/*
 * Tworzy folder/foldery.
 * Składnia nazwaFolderu/[nazwa kolejnego folderu]/
 * Jeżeli dany folder istnieje to zwraca 200 OK
 * Zwraca wyjątek tylko w przypadku braku uprawnień do tworzenia folderu.
 *
 */
void FTP::makeDirectory(string name) {
    Directory::createDirectories(name, currentDirectory);
    sendResponse("200 OK");
}

//file transfer methods
void FTP::setTransferType(string type) {
    if (type.size() != 1) {
        throw ServerException("501 Błąd w składni parametrów lub argumentów.");
    }

    //to uppercase
    if (type[0] >= 'a')
        type[0] -= 32;

    switch (type[0]) {
        case 'A':
            transferType = 'A';
            sendResponse("200 Type set to A."); //tryb ASCII
            break;
        case 'I':
            transferType = 'I';
            sendResponse("200 Type set to I."); //tryb binary
            break;
        default:
            throw ServerException("501 Niewspierany tryb pracy.");
    }
}

/*
 * Listuje pliki z podanego folderu.
 * dirName jest ścieżką względną. (Względem PWD).
 */
void FTP::listFiles(string dirName) {
    string list = Directory::listFiles(dirName, currentDirectory);
    sendResponse(list);
}

/*
 * Zmienia bieżący katalog.
 * Parametr jest ścieżką bezwzględną.
 *
 */
void FTP::changeDirectory(string name) {
    currentDirectory = Directory::changeDirectory(name);
    sendResponse("200 OK");
}

/*
 * Wypisuje aktualny katalog, przy czym
 * będąc w katalogu głównym wypisuje -> /
 * będą poza katalogiem głównym nie wypisują wiodącego /
 * znajdując się w folderze folder1 wypisze folder1/
 */
void FTP::printDirectory() {
    sendResponse(currentDirectory);
}


//example /dir1/named/ dir/dir2,  "named/ dir" -> "named dir"
string FTP::getDirectoryWithSpaces(vector<string> command) {
    //at index 0 is command string
    int iter = 1;
    string directory = command[iter];

    //checks if last char in a directory is a backslash that indicates a space
    while(command[iter][command[iter].size() - 1] == '\\')
    {
        //remove backslash from directory
        directory.erase(directory.size() - 1, 1);

        //check if there is another string to add
        if(command.size() - 1 >= (iter + 1))
        {
            directory += char(32);  //add space
            directory += command[iter+1];
        }
        else
        {
            break;
        }
        iter++;
    }
    if(directory[directory.size() - 1] != '/')
    {
        directory += '/';
    }
    return directory;
}


