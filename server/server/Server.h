#include "Directory.h"
#include "Client.h"
#include "FTP.h"
#include "ServerException.h"
#include "TerminalUtils.h"


//system includes
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <iostream>

#define BUFFER_SIZE 1000
#define QUEUE_SIZE 5

#define DEFAULT_PORT 10001
#define DEFAULT_ADDR "127.0.0.1"
/*
        RFC
        https://tools.ietf.org/html/rfc959
        http://www.cs.put.poznan.pl/mboron/prez/zasady_projektow.pdf

        Methods to implement:
                ascii
                binary
                mkdir
                rmdir
                put
                get

        Dopisać odbieranie i wysyłanie danych - binarnie i tekstowo.
        Dekodowanie poleceń ( + obsługa nieznanych poleceń - do tego też odpowiednia odpowiedź dla klienta) i formułowanie odpowiedzi.


*/
using namespace std;

struct server_opts
{
    char * addr;
    int port;
};

//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
    int socketDescriptor;
    struct sockaddr_in *remote;
};

void *startServer(void *server_opts);
void *connection(void *t_data);
void handleConnection(int connection_socket_descriptor, struct sockaddr_in *remote);
void parseCommand(string command);
int createServerThread(char * addr, int port);
void cleanRoutine(void *arg);
void displayRequest(int socketDescriptor, char * request);
