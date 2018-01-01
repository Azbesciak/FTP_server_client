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

string FTP::toUpper(string data) {
    std::transform(data.begin(), data.end(), data.begin(), ::toupper);
    return data;
}


void FTP::parseCommand(string command) {
    vector<string> splittedCommand = splitCommand(command);
    if(splittedCommand.size() == 0)
        throw new string("501 Syntax error in parameters.");
     if(splittedCommand[0].find("MKD") > 0)
    {
        if(splittedCommand.size() < 2)
            throw new string("501 Syntax error in arguments.");
        //makeDirectory(splittedCommand[1]);
        makeDirectory("asdas");
    }/*
    map<string, FTP::CommandAction>::const_iterator element = stringToFunction.find(splittedCommand[0]);
    if(element != stringToFunction.end())
    {
        if(splittedCommand.size() < 2)
            throw new string("501 Syntax error in arguments.");
        element->second(splittedCommand[1]);
    }*/
    else
    {
        throw new string("504 Command not implemented.");
    }
}

void FTP::sendInitialMessage() {
    sendResponse("Serwer działa.");
}


FTP::FTP(int socket) : stringToFunction(create_stringToFunctionMap()) {
/*FTP::FTP(int socket) { */
    this->socket = socket;
    //TODO fix mapping, now using simple if's
    //stringToFunction.insert(make_pair("MKD", &FTP::makeDirectory ));
}

void FTP::sendResponse(string message) {
    message += "\r\n";
    write(socket, message.c_str(), message.size());
}



vector<string> FTP::splitCommand(string command) {
    vector<string> pieces;
    istringstream iss(command);
    copy(istream_iterator<string>(),
         istream_iterator<string>(),
         back_inserter(pieces));
    return pieces;
}


map<string, FTP::CommandAction> FTP::create_stringToFunctionMap() {
        map<string, CommandAction > temp;
        temp.insert(pair<string, CommandAction >("MKD", &FTP::makeDirectory));
        return temp;
}

//logging in methods
void FTP::userCommand(string name) {
    sendResponse("200 Log in as a user!");
}

//directory methods
void FTP::removeDirectory(string name) {
    try {
        Directory::removeDirectory(name);
        sendResponse("200 OK");
    }catch(string errorMessage)
    {
        sendResponse(errorMessage);
    }
}
void FTP::makeDirectory(string name) {
    try {
        Directory::createDirectory(name);
        sendResponse("200 OK");
    }catch(string errorMessage)
    {
        sendResponse(errorMessage);
    }
}

//file transfer methods
void FTP::setTransferType(string type) {
    switch(type[0])
    {
        case 'A':
            transferType = 'A';
            break;
        case 'I':
            transferType = 'I';
            break;
        default:
            throw new string("501 Not supported transfer type!");
    }
    sendResponse("200 OK");
}

