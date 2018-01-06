//
// Created by jakub on 01.01.18.
//

#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <arpa/inet.h>

//first method to display interface addr
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h> //IFNAMSIZ

//second method
#include <fstream>

//my includes
#include "FTP.h"
#include "Directory.h"
#include "ServerException.h"
#include "TerminalUtils.h"

vector<uint16_t> FTP::dataConnectionPorts;

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
            throw ServerException("501 Brak oczekiwanego prametru.");
        }
        setTransferType(splittedCommand[1]);
    } else if (splittedCommand[0].find("MKD") != string::npos) {
        if (splittedCommand.size() < 2) {
            throw ServerException("501 Brak oczekiwanego prametru.");
        }
        string dirToCreate = getDirectoryWithSpaces(splittedCommand);
        makeDirectory(dirToCreate);
    } else if (splittedCommand[0].find("RMD") != string::npos) {
        if (splittedCommand.size() < 2) {
            throw ServerException("501 Brak oczekiwanego prametru.");
        }
        string directoryToRemove = getDirectoryWithSpaces(splittedCommand);
        removeDirectory(directoryToRemove);
    } else if (splittedCommand[0].find("LIST") != string::npos) {
        if (splittedCommand.size() < 2) {
            listFiles(currentDirectory);
        } else {
            listFiles(splittedCommand[1]);
        }
    } else if (splittedCommand[0].find("PWD") != string::npos) {
        //wypisz zawartrosc zmiennej currentDirectory
        printDirectory();
    } else if (splittedCommand[0].find("CWD") != string::npos) {
        if(splittedCommand.size() < 2)
        {
            changeDirectory("/");   //brak parametru, przejdz do glownego
        } else
        {
            changeDirectory(splittedCommand[1]);    //przejdz do wskazanego przez parametr
        }
    } else if (splittedCommand[0].find("PASSV") != string::npos) {
        sendPASSVResponse();
    } else if (splittedCommand[0].find("RETR") != string::npos) {
        //wysylanie plików z serwera do klienta
        if (splittedCommand.size() < 2) {
            throw ServerException("501 Brak oczekiwanego prametru.");
        }
        putFile(splittedCommand[1]);
    } else if (splittedCommand[0].find("STOR") != string::npos) {
        //wysylanie plikow od klineta na serwer
        if (splittedCommand.size() < 2) {
            throw ServerException("501 Brak oczekiwanego prametru.");
        }
        getFile(splittedCommand[1]);
    } else {
        throw ServerException("500 Komenda nierozpoznana.");
    }
}

FTP::FTP(int socket) : socketDescriptor(socket) {
    currentDirectory = "/";
    dataConnectionPort = 0;
}

void FTP::sendResponse(string message) {
    message += "\r\n";
    cout << "\t" << MAGENTA_TEXT("odpowiedź do " << socket << ":\t") << GREEN_TEXT(message);
    write(socketDescriptor, message.c_str(), message.size());
}


void FTP::putFile(string filename) {
    if(!uploadThreadActive) {
        createThread(ThreadType::Upload);
    }

    //TODO mutex

    fileToUpload = filename;

    //send 226 reply after sending data
}

void FTP::getFile(string filename) {
    throw ServerException("500 Niezaimplementowana komenda.");

}

//directory methods
void FTP::removeDirectory(string name) {
    Directory::removeDirectory(name, currentDirectory);
    sendResponse("250 OK");
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
    sendResponse("257 OK");
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
    string reply = "250 ";
    reply += currentDirectory;
    sendResponse(reply);
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

//rozbija komende na zbior wyrazen, co spacje
vector<string> FTP::splitCommand(string command) {
    vector<string> pieces;
    istringstream iss(command);
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter(pieces));
    return pieces;
}

//example /dir1/named/ dir/dir2,  "named/ dir" -> "named dir"
string FTP::getDirectoryWithSpaces(vector<string> command) {
    //at index 0 is command string
    size_t iter = 1;
    string directory = command[iter];

    //checks if last char in a directory is a backslash that indicates a space
    while (command[iter][command[iter].size() - 1] == '\\') {
        //remove backslash from directory
        directory.erase(directory.size() - 1, 1);

        //check if there is another string to add
        if (command.size() - 1 >= (iter + 1)) {
            directory += char(32);  //add space
            directory += command[iter+1];
        } else {
            break;
        }
        iter++;
    }
    if (directory[directory.size() - 1] != '/') {
        directory += '/';
    }
    return directory;
}

void FTP::sendPASSVResponse() {

    string defaultInterfaceAddr = getDefaultInterfaceAddr();
    string randomPort = getRandomPort();
    sendResponse("227 Entering Passive Mode (" + defaultInterfaceAddr + "," + randomPort + ")");
}

string FTP::getDefaultInterfaceName() {
    //TODO mutex
    system("ip route | awk '/default/ {printf $5}' >> eth");   //get default interface
    ifstream file("eth");
    string interfaceName;
    if (file.is_open()) {
        if (!file.eof()) {
            file >> interfaceName;
        }
        file.close();
        unlink("eth");
    }
    return interfaceName;
}

string FTP::getDefaultInterfaceAddr() {
    string interfaceName = getDefaultInterfaceName();

    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    // I want to get an IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;

    // I want IP address attached to "eth0"
    if (interfaceName.empty()) {
        interfaceName = "eth0";
    }
    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1); //enp0s3

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        interfaceName = "eth0";
#if DEBUG
        printf("error while finding interface\n");
#endif
    }
    close(fd);

    string addr(inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));

    size_t pos;
    while ((pos = addr.find(".")) != string::npos) {
        addr.replace(pos, 1, ",");
    }
#if false
    printf("error while finding interface\n");
#endif
    return addr;
}

bool FTP::isPortReserved(uint16_t port) {
    //check in list with ports
    for (auto const &value: dataConnectionPorts) {
        if (value == port) {
            return true;
        }
    }

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return true;
    }
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &sockAddr.sin_addr);
    sockAddr.sin_port = port;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bindowanie do socketu
    int time = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &time, sizeof(time));
    if (bind(sockfd, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        return true;
    }

    close(sockfd);

    return false;
}

string FTP::getRandomPort() {
    //TODO mutex
    uint16_t port;
    uint16_t p1;
    uint16_t p2;
    srand(static_cast<unsigned int>(time(nullptr)));
    do {
        //port = p1 * 256 + p2
        //p1 * 256  -> [1024, 32 768]
        //p2        -> [0, 32 767]
        p1 = ((rand() % 125) + 4); //p1 -> [4, 128]
        p2 = ((rand() % (1 << 15)) - 1);
        port = (p1 << 8) + p2;
    } while (isPortReserved(port));

    //add port to global ports
    FTP::dataConnectionPorts.push_back(port);

    string portStr;
    auto *temp = new char[20];
    sprintf(temp, "%d", p1);

    portStr = temp;
    portStr += ',';

    memset(temp, 0, 20);
    sprintf(temp, "%d", p2);
    portStr += temp;

    delete[]temp;
    return portStr;
}

int FTP::createThread(ThreadType threadType) {
    //tworzy watek dla serwera

    int create_result;
    switch(threadType)
    {
        case ThreadType::Download:
            //create_result = pthread_create(&downloadThreadHandle, nullptr, downloadThread, (void *)"sss");
            break;
        case ThreadType ::Upload:
            //create_result = pthread_create(&uploadThreadHandle, nullptr, uploadThread, (void *)"sss");
            break;
        default:
            throw new ServerException("Unknown ThreadType when creating data connection thread.");
    }

    if (create_result != 0) {
        throw new ServerException("Błąd przy próbie utworzenia wątku dla serwera, kod błędu: " + create_result);
    } else {
#if DEBUG
        cout << "Creted successfully " << (threadType == ThreadType::Upload ? " upload " : " download ")
             << "thread for client " << socketDescriptor << endl;
#endif
    }
    return create_result;
}

void *FTP::uploadThread(void *args) {

    uploadThreadActive = true; //TODO mutex



    uploadThreadActive = false;
}

void *FTP::downloadThread(void *args) {

    downloadThreadActive = true; //TODO mutex



    downloadThreadActive = false;}


