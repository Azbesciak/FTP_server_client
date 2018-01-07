#include "Server.h"
#include "ServerConfig.h"

int runserver = 1;
Client clients[MAX_THREADS + 1];
int currentClientNumber = 0;

int main(int argc, char *argv[]) {
    string command;

    auto *serverAddr = (char *) DEFAULT_ADDR;
    int port = argc == 2 ? atoi(argv[1]) : DEFAULT_PORT;

    int serverThread = createServerThread(serverAddr, port);

    while (command != "exit"
           && command != "quit") {
        cin >> command;
        parseCommand(command);
    }
    pthread_cancel(static_cast<pthread_t>(serverThread));
    return 0;
}

int createServerThread(char *addr, int port) {
    //uchwyt na wątek
    pthread_t serverThread;
    server_opts *serverOpts;
    serverOpts = new server_opts;
    serverOpts->addr = addr;
    serverOpts->port = port;
    //tworzy watek dla serwera
    int create_result = pthread_create(&serverThread, nullptr, startServer, (void *) serverOpts);
    if (create_result != 0) {
        printf("Błąd przy próbie utworzenia wątku dla serwera, kod błędu: %d\n", create_result);
    } else {
        cout << "Serwer wystartował. Folder główny: " << Directory::getRootDir() << "\n";
    }
    return create_result;
}


//strtok - rozbija string z delimiterem
void *startServer(void *serverOpts) {

    auto *options = (server_opts *) serverOpts;
    printf("Serwer FTP działa na adresie: %s:%d\n", options->addr, options->port);

    struct sockaddr_in sockAddr{};
    struct sockaddr_in remote{};

    int socketNum = socket(AF_INET, SOCK_STREAM, 0);
    if (socketNum < 0) {
        printf("Socket error\n");
        exit(-1);
    }
    //socket tylko uzywamy do czasu accept

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    inet_pton(AF_INET, options->addr, &sockAddr.sin_addr);
    sockAddr.sin_port = htons(static_cast<uint16_t>(options->port));
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bindowanie do socketu
    int time = 1;
    setsockopt(socketNum, SOL_SOCKET, SO_REUSEADDR, (char *) &time, sizeof(time));
    if (bind(socketNum, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        perror("Binding error");
        exit(-1);
    }


    if (listen(socketNum, QUEUE_SIZE) < 0) {
        perror("Listen error");
        exit(-1);
    }
    //pthread_cleanup_push(cleanRoutine, (void *) 1);
    socklen_t sockSize = sizeof(struct sockaddr);
    while (runserver > 0) {
        int connection_descriptor = accept(socketNum, (struct sockaddr *) &remote, &sockSize);
        if (connection_descriptor < 0) {
            perror("Client accepting error");
            runserver = 0;
            continue;
        }

        char remoteAddr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(remote.sin_addr), remoteAddr, INET_ADDRSTRLEN);
        //pass structure with client's data port
        printf("Podłączono klienta z adresem %s. Przypisany deskryptor %d\n", remoteAddr, connection_descriptor);

        if(currentClientNumber >= MAX_THREADS)
        {
            //dowidzenia
            string message = "500 Za dużo klientów.";
            write(connection_descriptor, message.c_str(), message.size());
            close(connection_descriptor);
            continue;
        }
        handleConnection(connection_descriptor, &remote);
    }
    //pthread_cleanup_pop(true);
    close(socketNum);
    exit(0);


}

//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor, struct sockaddr_in *remote) {
    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t clientThread;

    //dane, które zostaną przekazane do wątku
    //t_data jest usuwany po odlaczeniu sie klienta
    struct thread_data_t *t_data;
    t_data = new thread_data_t;
    t_data->socketDescriptor = connection_socket_descriptor;
    t_data->remote = remote;

    Client *newClient = new Client();
    newClient->socketDescriptor = connection_socket_descriptor;
    newClient->IPv4Data = remote;
    newClient->dataPort = 0;

    //tworzy watek dla nowego klienta
    create_result = pthread_create(&clientThread, nullptr, connection, (void *) t_data);
    if (create_result != 0) {
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
    }

    clients[currentClientNumber] = *newClient;
    pthread_mutex_lock(&currentClientNumber_mutex);
    currentClientNumber++;
    pthread_mutex_unlock(&currentClientNumber_mutex);
}

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *connection(void *t_data) {
    pthread_detach(pthread_self());
    auto *client = (struct thread_data_t *) t_data;

    char remoteAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client->remote->sin_addr), remoteAddr, INET_ADDRSTRLEN);

    cout << "Inicjalizacja się powiodła. Deskryptor " << GREEN_TEXT(client->socketDescriptor) << ", trafił z adresu "
         << GREEN_TEXT(remoteAddr) << ".\n";

    auto *buffer = new char[BUFFER_SIZE];
    int keepConnection = 1;
    auto *ftpClient = new FTP(client->socketDescriptor);

    //main client's loop
    while (keepConnection > 0) {
        ssize_t value = read(client->socketDescriptor, buffer, BUFFER_SIZE);
        if (value > 0) {
            displayRequest(client->socketDescriptor, buffer);
            try {
                ftpClient->parseCommand(buffer);
            } catch (ServerException &errorMessage) {
                ftpClient->sendResponse(errorMessage.what());
            } catch (...) {
                ftpClient->sendResponse("500 Nieznany problem.");
            }
        } else if (buffer[0] == 0) {
            cout << RED_TEXT("Klient z adresu " << remoteAddr << ", o deskryptorze " << client->socketDescriptor
                                                << " się rozłączył!\n");
            keepConnection = 0;
            continue;
        } else {
            printf("Nierozpoznana odpowiedź.\n");
        }
        //czyszczenie bufora, aby uniknac pomieszania z poprzednimi komendami
        memset(buffer, 0, BUFFER_SIZE);
    }
    delete ftpClient;
    pthread_mutex_lock(&currentClientNumber_mutex);
    currentClientNumber--;
    pthread_mutex_unlock(&currentClientNumber_mutex);

    pthread_exit(nullptr);
}


void parseCommand(string command) {
    if (command.find("quit") != string::npos || command.find("quit") != string::npos) {
        cout << GREEN_TEXT("Zamykanie serwera.\n");
        runserver = 0;
    } else if (command.find("list") != string::npos) {
        int iter = 1;
        for (auto const &value: clients) {
            if (value.socketDescriptor > 0) {
                char remoteAddr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(value.IPv4Data->sin_addr), remoteAddr, INET_ADDRSTRLEN);

                cout << iter++ << ". Klient, socket deskryptor: " << value.socketDescriptor
                     << ", IP:" << remoteAddr << endl;
            }
        }
        if (iter == 1) {
            cout << RED_TEXT("Brak klientów") << endl;
        }

    } else {
        cout << RED_TEXT("Brak zdefiniowanej funkcji dla ") << WHITE_TEXT(command) << "\n";
    }
}


void displayRequest(int socketDescriptor, char *request) {
    cout << YELLOW_TEXT("Klient " << socketDescriptor) << "\n";
    cout << "\t" << MAGENTA_TEXT("Zapytanie od " << socketDescriptor << ":\t") << GREEN_TEXT(request);
}


