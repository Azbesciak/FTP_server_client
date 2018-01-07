//
// Created by jakub on 01.01.18.
//

#ifndef SERVER_FTP_H
#define SERVER_FTP_H
#include <map>
#include <vector>
#include <string>
#include "Client.h"

using namespace std;

class FTP {
public:
    FTP();
    explicit FTP(int socket);   //always use to proper initialization
    FTP(Client *client);
    ~FTP();
    void parseCommand(string command);
    void parseCommand(char * command);
    void sendResponse(string message);
    void killDataConnectionThreads();
private:
    Client *clientData;
    vector<string> splitCommand(string command);
    string toUpper(string data);

    int socketDescriptor;
    //directory stuff
    void makeDirectory(string name);
    void removeDirectory(string name);
    void printDirectory();
    void changeDirectory(string name);


    //file transfer stuff
    void setTransferType(string type);
    void putFile(string filename);
    void getFile(string filename);

    void listFiles(string dirName);

    //A = ASCII
    //I = Binary
    char transferType = 0;

    string currentDirectory;
    string getDirectoryWithSpaces(vector<string> command);

    //PASSV command
    void sendPASSVResponse();

    string getDefaultInterfaceName();
    string getDefaultInterfaceAddr();
    bool isPortReserved(uint16_t port);
    string getRandomPort();
    static vector<uint16_t> dataConnectionPorts;
    uint16_t dataConnectionPort;

    //threads
        //used both in upload and download thread
    struct sockaddr_in remote{};
    bool dataConnectionOpened;
    int dataConnectionSocket;
    enum ThreadType
    {
        Download,
        Upload
    };
    //controlled by threads
    bool uploadThreadActive = false;
    bool downloadThreadActive = false;

    //controlled by instance in main client's thread
    bool tryToDownloadFile = false;
    bool tryToUploadFile = false;

    pthread_t downloadThreadHandle = 0;
    pthread_t uploadThreadHandle = 0;
    string fileToUpload;
    string fileToDownload;
    int createThread(ThreadType threadType);
    void *uploadThread(void *args);
    void *downloadThread(void *args);
    //wrappers to threads
    static void* newUploadThreadWrapper(void *object);
    static void *newDownloadThreadWrapper(void *object);

    void prepareFileToDownload();

    void setUpSocketForDataConnection();
};


#endif //SERVER_FTP_H
