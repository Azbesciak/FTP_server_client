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
    int socket;
    string toUpper(string data);

    //directory stuff
    void makeDirectory(string name);
    void removeDirectory(string name);
    void printDirectory();
    void changeDirectory(string name);


    //file transfer stuff
    void setTransferType(string type);
    void uploadFile(string filename);
    void downloadFile(string filename);

    void listFiles(string dirName);

    //A = ASCII
    //I = Binary
    char transferType = 0;

    int dataTransferPort;
    string currentDirectory;
    string getDirectoryWithSpaces(vector<string> command);
};


#endif //SERVER_FTP_H
