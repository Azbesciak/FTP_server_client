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
    FTP(int socket);
    typedef void (FTP::*CommandAction)(string arg);

    void sendInitialMessage();
    void parseCommand(string command);
    void parseCommand(char * command);
    void sendResponse(string message);
    char *buffer;
private:
    vector<string> splitCommand(string command);
    int socket;
    string toUpper(string data);
    void userCommand(string name);

    map<string, FTP::CommandAction>  create_stringToFunctionMap();
    const map<string, FTP::CommandAction> stringToFunction ;

    //directory stuff
    void makeDirectory(string name);
    void removeDirectory(string name);


    //file transfer stuff
    void setTransferType(string type);
    void uploadFile(string filename);
    void downloadFile(string filename);


    //A = ASCII
    //I = Binary
    char transferType = 0;

    int dataTransferPort;


};


#endif //SERVER_FTP_H
