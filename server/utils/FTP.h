//
// Created by jakub on 01.01.18.
//

#ifndef SERVER_FTP_H
#define SERVER_FTP_H
#include <map>
#include <vector>
#include <string>
using namespace std;

class FTP {
public:
    explicit FTP(int socket);
    void parseCommand(string command);
    void parseCommand(char * command);
    void sendResponse(string message);
private:
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

    int dataTransferPort;
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
    enum ThreadType
    {
        Download,
        Upload
    };
    bool uploadThreadActive = false;
    bool downloadThreadActive = false;
    pthread_t downloadThreadHandle;
    pthread_t uploadThreadHandle;
    string fileToUpload;
    string fileToDownload;
    int createThread(ThreadType threadType);
    void *uploadThread(void *args);
    void *downloadThread(void *args);
};


#endif //SERVER_FTP_H
