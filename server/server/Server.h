#include "Directory.h"
#include "Client.h"
#include "FTP.h"
#include "ServerException.h"
#include "TerminalUtils.h"


//system includes
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <dirent.h>
#include <iostream>

/*
        RFC
        https://tools.ietf.org/html/rfc959
        http://www.cs.put.poznan.pl/mboron/prez/zasady_projektow.pdf
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

pthread_mutex_t currentClientNumber_mutex = PTHREAD_MUTEX_INITIALIZER;


void *startServer(void *server_opts);
void *connection(void *t_data);
void handleConnection(int connection_socket_descriptor, struct sockaddr_in *remote);
void parseCommand(string command);
int createServerThread(char * addr, int port);
void displayRequest(int socketDescriptor, char * request);
