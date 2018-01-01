//
// Created by jakub on 01.01.18.
//

#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include <netinet/in.h>

class Client {
public:
    Client();
    ~Client();
    int socketDescriptor;
    int dataPort;
    struct sockaddr_in *IPv4Data;
};


#endif //SERVER_CLIENT_H
