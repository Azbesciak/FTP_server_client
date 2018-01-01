//
// Created by jakub on 02.01.18.
//

#ifndef SERVER_SERVEREXCEPTION_H
#define SERVER_SERVEREXCEPTION_H

#include <exception>
#include <string>
using namespace std;
class ServerException : exception {
public:
    ServerException(std::string stringStream) : message(stringStream) {}
    ~ServerException() throw () {}
    const char* what() const throw() { return message.c_str(); }
    const string what() { return message;}
private:
    string message;
};

#endif //SERVER_SERVEREXCEPTION_H
