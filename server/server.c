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

#define BUFFER_SIZE 1000
#define MAX_NUMBERS_TO_PARSE 10
#define QUEUE_SIZE 5

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

*/



void parseReceivedData(char *receivedData, int *numbers, int *numbersSize);
int sumReceivedData(int *numbers, int numbersSize);
int startServer(char * addr, int port);
void *ThreadBehavior(void *t_data);
void handleConnection(int connection_socket_descriptor, struct sockaddr_in *remote);
int sendData(int socketNum, char *message);


//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
    int socektDescriptor;
    struct sockaddr_in *remote;
};


int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		startServer("127.0.0.1", 10001);		
	}
	else if(argc == 2)
	{
		startServer("127.0.0.1", atoi(argv[1]));
	}
	else if(startServer(argv[1], atoi(argv[2])) < 0)
	{
		printf("Serwer nie wystartowal.\n");
		exit(0);
	}
	else
	{
		printf("Argc error");

	}

		
	return 0;
}


//strtok - rozbija string z delimiterem
int startServer(char * addr, int port)
{
	printf("Server: %s:%d\n", addr, port);
	char buff[BUFFER_SIZE];
	struct sockaddr_in sockAddr, remote;
	
        int socketNum = socket(AF_INET, SOCK_STREAM, 0);
        if(socketNum < 0)
        {
                printf("Socket error\n");
                return -1;
        }
	//socket tylko uzywamy do czasu accept

	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	inet_pton(AF_INET, addr, &sockAddr.sin_addr);
	sockAddr.sin_port = htons(port);
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//bindowanie do socketu
	int time = 1;
	setsockopt(socketNum, SOL_SOCKET, SO_REUSEADDR, (char*)&time, sizeof(time));
	if(bind(socketNum, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0)
	{
		perror("Binding blad:");
		return -1;
	}



	if(listen(socketNum, QUEUE_SIZE) < 0)
	{
		perror("Blad listen:");
		return -1;
	}
	
	int sockSize = sizeof(struct sockaddr);

	int runserver = 1;

	while(runserver > 0)
	{
		int connection_descriptor = accept(socketNum, (struct sockaddr *) &remote, &sockSize);
		if(connection_descriptor < 0)
		{
			perror("Accept client error");
			runserver = 0;
			continue;
		}

		int readBytes = read(connection_descriptor, buff, BUFFER_SIZE);
		char remoteAddr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(remote.sin_addr), remoteAddr, INET_ADDRSTRLEN);
		//		int remotePort = 2;//ntohs(remote.sin_port);
		printf("Client %s connected. Assigned socket %d\n", remoteAddr, connection_descriptor);	 
		
		handleConnection(connection_descriptor, &remote);
	}

	close(socketNum);
    return 0;


}
//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor, struct sockaddr_in *remote ) {
    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    //TODO dynamiczne utworzenie instancji struktury thread_data_t o 
    //	nazwie t_data (+ w odpowiednim miejscu zwolnienie pamięci)
    //TODO wypełnienie pól struktury
    struct thread_data_t *t_data;
    t_data = malloc((sizeof(struct thread_data_t)));
    t_data->socektDescriptor = connection_socket_descriptor;
    t_data->remote = remote;

    char buffor[BUFFER_SIZE];

    //tworzy watek dla nowego klienta
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }
    else
    {
    	printf("Creating thread success\n");
    }

}

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data)
{
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    char remoteAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(th_data->remote->sin_addr), remoteAddr, INET_ADDRSTRLEN);

	printf("Listener for %s created!\n", remoteAddr);
	char buffer[BUFFER_SIZE];
	int keepConnection = 1;
	while(keepConnection > 0)
	{
		if(read(th_data->socektDescriptor, buffer, BUFFER_SIZE) > 0)
		{
			printf("Received data from %s: %s\n", remoteAddr, buffer);
			sendData(th_data->socektDescriptor, "data");
		}
	}
    //dostęp do pól struktury: (*th_data).pole
    //TODO (przy zadaniu 1) klawiatura -> wysyłanie albo odbieranie -> wyświetlanie
    printf("Thread created. Socket number is %d, addrs %s\n", th_data->socektDescriptor, remoteAddr);
    pthread_exit(NULL);
}

int sendData(int socketNum, char *message)
{
	printf("data to send %s\n");
	char dataToSend[20];
	sprintf(dataToSend, "%s",message);

	return write(socketNum, message, sizeof(dataToSend));
//	printf("Send:%s.\nserver: %s, port: %d.\n%d %d\n", dataToSend, addr, port, dataSend, sizeof(dataToSend));	

}

void parseReceivedData(char *receivedData, int *numbers, int *numbersSize)
{
	*numbersSize = 0;
	int iter = 0;
	char *str = strtok(receivedData, ";");
	while(str != NULL && iter < MAX_NUMBERS_TO_PARSE)
	{
		numbers[iter] = atoi(str);
		iter++;
		str = strtok(NULL, ";");
	}
	*numbersSize = iter;
}
int sumReceivedData(int *numbers, int numbersSize)
{
	int result = 0;
	for(int i=0; i<numbersSize;i++)
	{
		result += numbers[i];
	}
	return result;
}
