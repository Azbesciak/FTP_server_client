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
        throw new ServerException("500 Command line too long.");
    }

    vector<string> splittedCommand = splitCommand(command);
    if (splittedCommand.size() == 0) {
        throw new ServerException("500 Syntax error.");
    }
    int pos = 0;
    splittedCommand[0] = toUpper(splittedCommand[0]);
    if ((pos = splittedCommand[0].find("TYPE")) >= 0) {
        if (splittedCommand.size() < 2) {
            throw new ServerException("501 Syntax error in parameters.");
        }
        setTransferType(splittedCommand[1]);
    } else if ((pos = splittedCommand[0].find("MKDIR")) >= 0) {
        if (splittedCommand.size() < 2) {
            throw new ServerException("501 Syntax error in parameters.");

        }
        makeDirectory(splittedCommand[1]);
    } else if ((pos = splittedCommand[0].find("RMDIR")) >= 0) {
        if (splittedCommand.size() < 2) {
            throw new ServerException("501 Syntax error in parameters.");
        }
        removeDirectory(splittedCommand[1]);
    } else if ((pos = splittedCommand[0].find("LIST")) >= 0) {
        if (splittedCommand.size() < 2) {
            listFiles(currentDirectory);
        }
        listFiles(splittedCommand[1]);
    } else {
        throw new ServerException("500 Command unrecognized.");
    }
}

void FTP::sendInitialMessage() {
    ;//sendResponse("Serwer działa.");
}


FTP::FTP(int socket) : stringToFunction(create_stringToFunctionMap()) {
/*FTP::FTP(int socket) { */
    this->socket = socket;
    currentDirectory = "/";
    //TODO fix mapping, now using simple if's
    //stringToFunction.insert(make_pair("MKD", &FTP::makeDirectory ));
}

void FTP::sendResponse(string message) {
    message += "\r\n";
    cout << "\t" << MAGENTA_TEXT("response to " << socket << ":\t") << GREEN_TEXT(message);
    write(socket, message.c_str(), message.size());
}


vector<string> FTP::splitCommand(string command) {
    vector<string> pieces;
    istringstream iss(command);
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter(pieces));
    return pieces;
}


map<string, FTP::CommandAction> FTP::create_stringToFunctionMap() {
    map<string, CommandAction> temp;
    temp.insert(pair<string, CommandAction>("MKD", &FTP::makeDirectory));
    return temp;
}

//directory methods
void FTP::removeDirectory(string name) {
    Directory::removeDirectory(name);
    sendResponse("200 OK");
}

//throws ServerException if
void FTP::makeDirectory(string name) {
    Directory::createDirectories(name);
    sendResponse("200 OK");
}

//file transfer methods
void FTP::setTransferType(string type) {
    if (type.size() != 1) {
        throw new ServerException("501 Syntax error in parameters or arguments.");
    }

    //to uppercase
    if (type[0] >= 'a')
        type[0] -= 32;

    switch (type[0]) {
        case 'A':
            transferType = 'A';
            sendResponse("200 Type set to A.");
            break;
        case 'I':
            transferType = 'I';
            sendResponse("200 Type set to I.");
            break;
        default:
            throw new ServerException("501 Not supported transfer type!");
    }
}

void FTP::listFiles(string dirName) {
    string list = Directory::listFiles(dirName);
    sendResponse(list);
}


