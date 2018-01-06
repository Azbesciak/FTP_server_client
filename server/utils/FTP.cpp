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
#include "ServerConfig.h"

//my includes
#include "FTP.h"
#include "Directory.h"
#include "ServerException.h"
#include "TerminalUtils.h"

//static fields
vector<uint16_t> FTP::dataConnectionPorts;

FTP::FTP() {
    throw new ServerException("Use FTP(int socket) instead.");
}

FTP::FTP(int socket) : socketDescriptor(socket) {
    currentDirectory = "/";
    dataConnectionPort = 0;
    dataConnectionOpened = false;
}

string FTP::toUpper(string data) {
    std::transform(data.begin(), data.end(), data.begin(), ::toupper);
    return data;
}

void FTP::parseCommand(char *command) {
    string data(command);
    this->parseCommand(data);
}

void FTP::parseCommand(string command) {
    if (command.size() > 280) {
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
            string directory = getDirectoryWithSpaces(splittedCommand);
            listFiles(directory);
        }
    } else if (splittedCommand[0].find("PWD") != string::npos) {
        //wypisz zawartrosc zmiennej currentDirectory
        printDirectory();
    } else if (splittedCommand[0].find("CWD") != string::npos) {
        if (splittedCommand.size() < 2) {
            changeDirectory("/");   //brak parametru, przejdz do glownego
        } else {
            string directory = getDirectoryWithSpaces(splittedCommand);
            changeDirectory(directory);    //przejdz do wskazanego przez parametr
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


void FTP::sendResponse(string message) {
    message += "\r\n";
    cout << "\t" << MAGENTA_TEXT("odpowiedź do " << socket << ":\t") << GREEN_TEXT(message);
    write(socketDescriptor, message.c_str(), message.size());
}


void FTP::putFile(string filename) {
    if (dataConnectionPort == 0) {
        throw ServerException("500 Send PASSV.");
    }
    throw ServerException("500 Niezaimplementowana komenda.");

}

void FTP::getFile(string filename) {
    if (dataConnectionPort == 0) {
        throw ServerException("500 Send PASSV.");
    }
    if (!downloadThreadActive) {
        createThread(ThreadType::Download);
    }

    //TODO mutex

    fileToDownload = filename;


    //send 226 reply after sending data
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
 * list /   -> listuje katalog głowny
 * list     -> listuje aktualny katalog
 * list dir -> listuje podkatalog dir
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
    currentDirectory = Directory::changeDirectory(name, currentDirectory);
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
            directory += command[iter + 1];
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

/*
 * Jeżeli jest wysyłany drugi passv to staramy sie zamknac poprzedni port,
 * jeżeli jest to niemozliwe, rzucamy wyjatek.
 *
 */
void FTP::sendPASSVResponse() {
    //TODO mutex for socketDescriptor and activity flags
    //TODO wyslac jakies gowno jezeli jest passv kiedy juz byl ustawiony
    if (uploadThreadActive || downloadThreadActive) {
        throw ServerException("500 0,0 Zmiana portu niemożliwa, port aktualnie w użyciu.");
    } else {
        if (dataConnectionSocket != 0) {
            dataConnectionOpened = false;
            close(dataConnectionSocket);
        }
        //zabij watki, mozliwe, ze moga dalej sie starac otwierac poprzedni socket
        killDataConnectionThreads();
    }

    string defaultInterfaceAddr = getDefaultInterfaceAddr();
    string randomPort = getRandomPort();

    //kiedy port ustawiony otworz polaczenie, aby zagwarantowac, ze zaden inny program
    //nie odbierze nam portu
    setUpSocketForDataConnection();

    sendResponse("227 " + randomPort + ".");
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
        interfaceName = DEFAULT_INTERFACE;
    }
    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1); //enp0s3

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        interfaceName = DEFAULT_INTERFACE;
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
    inet_pton(AF_INET, DEFAULT_ADDR, &sockAddr.sin_addr);
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
    //set port in this instance
    dataConnectionPort = port;


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
    switch (threadType) {
        case ThreadType::Download:
            create_result = pthread_create(&downloadThreadHandle, nullptr, newDownloadThreadWrapper, this);
            break;
        case ThreadType::Upload:
            create_result = pthread_create(&uploadThreadHandle, nullptr, newUploadThreadWrapper, this);
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

//TODO mutexex
/*
 * Mutex:
 *      initialization -> when connection is not open
 *      before file creation and when opening file
 */
void *FTP::uploadThread(void *args) {

    uploadThreadActive = true; //TODO mutex
    //TODO mutex on binding
    if (!dataConnectionOpened) {
        setUpSocketForDataConnection();
    }

    //wait for connection from client
    cout << "Oczekiwanie na połączenie na porcie " << dataConnectionPort << endl;
    socklen_t sockSize = sizeof(struct sockaddr);
    int connection_descriptor = accept(socketDescriptor, (struct sockaddr *) &remote, &sockSize);
    if (connection_descriptor < 0) {
        perror("Client accepting error");
        dataConnectionOpened = 0;

    }

    //zapisnie adresu
    char remoteAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(remote.sin_addr), remoteAddr, INET_ADDRSTRLEN);
#if DEBUG
    cout << "Upload thread: Podłączono klienta z adresem" << remoteAddr << "Przypisany deskryptor"
         << connection_descriptor << endl;
#endif

    fstream file;
    if (transferType == 'A') {
        file.open(fileToUpload, ios::out);
    } else {
        file.open(fileToUpload, ios::out | ios::binary);
    }
#if DEBUG
    cout << "Upload thread: Przygotowywanie pliku do wyslania " << fileToUpload << endl;
#endif
    bool connectionOpened = true;

    //buffer for data
    auto *buffer = new char[BUFFER_SIZE];

    while (connectionOpened) {
        //while there are not sent bits...
        connectionOpened = false;
        cout << "Data sent" << endl;
    }

    file.close();

    //TODO mutex on socket descriptor
    if (!downloadThreadActive) {
        //close socket only when data is not being downloaded
        if (dataConnectionSocket != 0) {
            close(dataConnectionSocket);
            dataConnectionOpened = false;
        }
    }

    uploadThreadActive = false;
}

void *FTP::downloadThread(void *args) {

    downloadThreadActive = true; //TODO mutex
    if (!dataConnectionOpened) {
        setUpSocketForDataConnection();
    }

    //wait for connection from client
    cout << "Oczekiwanie na połączenie na porcie " << dataConnectionPort << endl;
    socklen_t sockSize = sizeof(struct sockaddr);
    int connection_descriptor = accept(dataConnectionSocket, (struct sockaddr *) &remote, &sockSize);
    if (connection_descriptor < 0) {
#if DEBUG
        cout << "Download thread, client " << socketDescriptor << " accepting error.\n";
#endif
        perror("Download thread.Client accepting error");
        dataConnectionOpened = 0;
    }

    //zapisnie adresu
    char remoteAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(remote.sin_addr), remoteAddr, INET_ADDRSTRLEN);
#if DEBUG
    cout << "Download thread: Podłączono klienta z adresem" << remoteAddr << "Przypisany deskryptor"
         << connection_descriptor << endl;
#endif

    //add current directory
    prepareFileToDownload();

    fstream file;
    if (transferType == 'A') {
        file.open(fileToDownload, ios::out);
    } else {
        file.open(fileToDownload, ios::out | ios::binary);
    }
#if DEBUG
    cout << "Download thread: Tworzenie pliku " << fileToUpload << endl;
#endif
    bool connectionOpened = true;

    //buffer for data
    auto *buffer = new char[BUFFER_SIZE];

    while (connectionOpened) {
        ssize_t value = read(dataConnectionSocket, buffer, BUFFER_SIZE);
        //check for EOT end of tranmission
        if (value == EOF) {
            connectionOpened = false;
            continue;
        }
        if (buffer[value - 1] == EOF) {
            file.write(buffer, value - 1);
            connectionOpened = false;
            continue;
        }
        file.write(buffer, value);
    }

    file.close();

    //TODO mutex on socket descriptor
    if (!uploadThreadActive) {
        //close socket only when data is not being downloaded
        if (dataConnectionSocket != 0) {
            close(dataConnectionSocket);
            dataConnectionOpened = false;
        }
    }

    downloadThreadActive = false;
}

void *FTP::newUploadThreadWrapper(void *object) {
    reinterpret_cast<FTP *>(object)->uploadThread(nullptr);
    return 0;
}

void *FTP::newDownloadThreadWrapper(void *object) {
    reinterpret_cast<FTP *>(object)->downloadThread(nullptr);
    return 0;
}

/*
 * Removes slash at the beginning.
 * Adds current directory position.
 */
void FTP::prepareFileToDownload() {
    //TODO mutex on fileToUpload
    Directory::slashesConverter(&fileToDownload);
    //remove slash at 0 position
    if (fileToDownload[0] == '/') {
        fileToDownload.erase(fileToDownload.size() - 1, 1);
    }

    //add current directory
    fileToDownload = currentDirectory + fileToDownload;
}

//initiate socket for dataconnection
void FTP::setUpSocketForDataConnection() {
    //open new socket for the connection
    struct sockaddr_in sockAddr;

    dataConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataConnectionSocket < 0) {
        printf("Client data connection socket. Socket error\n");
        throw ServerException("500 Internal server exception. Socket.");
    }
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    inet_pton(AF_INET, DEFAULT_ADDR, &sockAddr.sin_addr);
    sockAddr.sin_port = dataConnectionPort;
    cout << "Data conn port " << dataConnectionPort << endl;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bindowanie do socketu
    int time = 1;
    setsockopt(dataConnectionSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &time, sizeof(time));
    if (bind(dataConnectionSocket, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        perror("Client data connection socket. Binding error");
        throw ServerException("500 Internal server exception. Bind.");
    }

    if (listen(dataConnectionSocket, QUEUE_SIZE) < 0) {
        perror("Client data connection socket. Listen error");
        throw ServerException("500 Internal server exception. Listen.");
    }
#if DEBUG
    cout << "Client " << socketDescriptor << " socket for data connection initialized. Port " << dataConnectionPort << " binded.\n";
#endif
    dataConnectionOpened = true;
}

void FTP::killDataConnectionThreads() {
#if DEBUG
    cout << "Killing client's threads. Client descriptor " << socketDescriptor << endl;
#endif

    if (uploadThreadHandle != 0) {
#if DEBUG
        cout << "Client " << socketDescriptor << " upload thread killed"<< endl;
#endif
        pthread_cancel(uploadThreadHandle);
    }

    if (uploadThreadHandle != 0) {
#if DEBUG
        cout << "Client " << socketDescriptor << " download thread killed"<< endl;
#endif
        pthread_cancel(downloadThreadActive);
    }


}

