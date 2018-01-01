#include "server.h"

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


int runserver = 1;

int main(int argc, char *argv[])
{
	string command = "";


	char * serverAddr = argc == 3 ? argv[1] : (char*)DEFAULT_ADDR;
	int port = argc == 2 ? atoi(argv[1]) : (argc == 3 ? atoi(argv[2]) : DEFAULT_PORT);
	
	int serverThread = createServerThread(serverAddr, port);

	while(command != "exit" 
		&& command != "quit")
	{
		cin >> command;
		
	}


    pthread_cancel(serverThread);
	return 0;
}

int createServerThread(char * addr, int port)
{
    //uchwyt na wątek
    pthread_t serverThread;
	server_opts *serverOpts;
	serverOpts = new server_opts;
    serverOpts->addr = addr;
    serverOpts->port = port;
    //tworzy watek dla serwera
    int create_result = pthread_create(&serverThread, NULL, startServer, (void *)serverOpts);
    if (create_result != 0){
       printf("Błąd przy próbie utworzenia wątku dla serwera, kod błędu: %d\n", create_result);
    }
    else
    {
        printf("Server started...\n");
    }
    return create_result;
}

void cleanRoutine(void *arg)
{
    cout << "Cleaning routine\n";
}
//strtok - rozbija string z delimiterem
void *startServer(void *serverOpts) {


        server_opts *options = (server_opts *) serverOpts;
        printf("Server is running: %s:%d\n", options->addr, options->port);

        struct sockaddr_in sockAddr, remote;

        int socketNum = socket(AF_INET, SOCK_STREAM, 0);
        if (socketNum < 0) {
            printf("Socket error\n");
            exit(-1);
        }
        //socket tylko uzywamy do czasu accept

        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        inet_pton(AF_INET, options->addr, &sockAddr.sin_addr);
        sockAddr.sin_port = htons(options->port);
        sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        //bindowanie do socketu
        int time = 1;
        setsockopt(socketNum, SOL_SOCKET, SO_REUSEADDR, (char *) &time, sizeof(time));
        if (bind(socketNum, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
            perror("Binding blad:");
            exit(-1);
        }


        if (listen(socketNum, QUEUE_SIZE) < 0) {
            perror("Blad listen:");
            exit(-1);
        }
        pthread_cleanup_push(cleanRoutine, (void *)1);
        socklen_t sockSize = sizeof(struct sockaddr);
        while (runserver > 0) {
            int connection_descriptor = accept(socketNum, (struct sockaddr *) &remote, &sockSize);
            //printf("connection descriptor %d\n", connection_descriptor);
            if (connection_descriptor < 0) {
                perror("Client accepting error");
                runserver = 0;
                continue;
            }

            char remoteAddr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(remote.sin_addr), remoteAddr, INET_ADDRSTRLEN);
            //TODO pass structure with client's data port
            printf("Client %s connected. Assigned socket %d\n", remoteAddr, connection_descriptor);
            handleConnection(connection_descriptor, &remote);
        }
        pthread_cleanup_pop(1);
        close(socketNum);
        exit(0);


    }
//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor, struct sockaddr_in *remote ) {
    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    //TODO dynamiczne utworzenie instancji struktury thread_data_t o 
    //TODO wypełnienie pól struktury
    struct thread_data_t *t_data;
    t_data = new thread_data_t;	
    t_data->socketDescriptor = connection_socket_descriptor;
    t_data->remote = remote;

    //tworzy watek dla nowego klienta
    create_result = pthread_create(&thread1, NULL, connection, (void *) t_data);
    if (create_result != 0){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }
    else
    {
    	printf("Creating thread success\n");
    }

}

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *connection(void *t_data)
{
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    char remoteAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(th_data->remote->sin_addr), remoteAddr, INET_ADDRSTRLEN);

	printf("Thread created. Socket number is %d, remote host address is %s\n", th_data->socketDescriptor, remoteAddr);

	char buffer[BUFFER_SIZE];
	int keepConnection = 1;
	while(keepConnection > 0)
	{
		int value = read(th_data->socketDescriptor, buffer, BUFFER_SIZE);
        cout << "Reading...\n";
		if(value > 0)
		{
            /*
			if(sendExit > 0)
			{
				write(th_data->socketDescriptor, (const void *)-1,(size_t)1);
				printf("disconnected from client\n\n");
				keepConnection = 0;
				continue;
			}
			printf("Received data from %s: %s\n", remoteAddr, buffer);
            */

		}else if(buffer[0] == 0)
		{
				printf("Client %s disconnected.\n", remoteAddr);
				keepConnection = 0;
				continue;
		}else
		{
			printf("Undefined behaviour\n");
		}
		sleep(200);
		
	}
    delete (struct thread_data_t*)t_data;
    pthread_exit(NULL);
}


int sendResponse(int socketNum, char *message, int messageSize)
{

    string data = message;
    data = data;
	int value = write(socketNum, data.c_str(), data.size());
	if(value < 0)
	{
		printf("Error when sending data %s, socket number : %d\n", data.c_str(), socketNum);
		perror("Couldn't send data");
	}else
	{
		printf("Send %d bytes.\n", value);
	}
	return value;

}

void parseCommand(string command) {
    if(command.find("restart") > 0)
    {
        cout << "Restarting server";
    }
}
