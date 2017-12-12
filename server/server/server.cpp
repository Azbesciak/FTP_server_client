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

#include "directory.h"

#define BUFFER_SIZE 1000
#define MAX_NUMBERS_TO_PARSE 10
#define QUEUE_SIZE 5
#define FILTER_NOT_ALLOWED_FILES 4
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
typedef void (* CommandAction)(int);


//returns action appropriate to client's command
CommandAction * parseReceivedData(char *receivedData);	
int checkCommand(char *data, char *command);
int sumReceivedData(int *numbers, int numbersSize);
int startServer(char * addr, int port);
void *ThreadBehavior(void *t_data);
void handleConnection(int connection_socket_descriptor, struct sockaddr_in *remote);
int sendData2(int socketNum, char *message);
int sendResponse(int socketNum, char *message, int messageSize);
int readFile(char *filename, char * buffer, long *bufferSize);
void listFiles();

//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
    int socketDescriptor;
    struct sockaddr_in *remote;
};


int main(int argc, char *argv[])
{
	listDirFiles();	
	return -1;
	if(argc == 1)
	{
		startServer("127.0.0.1", 21);		
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
	printf("Server is running: %s:%d\n", addr, port);

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
	
	socklen_t sockSize = sizeof(struct sockaddr);

	int runserver = 1;

	while(runserver > 0)
	{
		int connection_descriptor = accept(socketNum, (struct sockaddr *) &remote, &sockSize);
		//printf("Connection descriptor %d\n", connection_descriptor);
		if(connection_descriptor < 0)
		{
			perror("Client accepting error");
			runserver = 0;
			continue;
		}
		//sendData(connection_descriptor, "220: Hello world\n\r", 30);

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
    t_data = new thread_data_t;	
    t_data->socketDescriptor = connection_socket_descriptor;
    t_data->remote = remote;

    //tworzy watek dla nowego klienta
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
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
void *ThreadBehavior(void *t_data)
{
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    char remoteAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(th_data->remote->sin_addr), remoteAddr, INET_ADDRSTRLEN);

	printf("Thread created. Socket number is %d, addrs %s\n", th_data->socketDescriptor, remoteAddr);


	char buffer[BUFFER_SIZE];
	int keepConnection = 1;
	int sendExit = 0;
	while(keepConnection > 0)
	{
		int value = read(th_data->socketDescriptor, buffer, BUFFER_SIZE);
		if(value > 0)
		{
			if(sendExit > 0)
			{
				write(th_data->socketDescriptor, (const void *)-1,(size_t)1);
				printf("disconnected from client\n\n");
				keepConnection = 0;
				continue;
			}
			printf("Received data from %s: %s\n", remoteAddr, buffer);

			char *buffer = NULL;
			char * filename = "file.bin";
			long bufferSize = 0;
			if(readFile(filename, buffer, &bufferSize))
			{
				printf("File, read %ld bytes.\n", bufferSize);
				buffer = "asdasd";
				//buffer[0] = buffer[1] = buffer[2] = buffer[3] = (char)0;
				//printf("Buffer[0] >%d<\n", buffer[0]);
				sendResponse(th_data->socketDescriptor, buffer, 100);
			}else
			{
				perror("Unable to read file");
				return 0;

			}
			//CommandAction * commandAction =  parseReceivedData("CWD");
			//if(commandAction != NULL)
		//		(*commandAction)("");
		//	else
		//		printf("No action defined\n");

			//sendData(th_data->socketDescriptor, "data");
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
		sendExit = 1;
		
	}
    pthread_exit(NULL);
}


int sendResponse(int socketNum, char *message, int messageSize)
{
	int value = write(socketNum, message, strlen(message));
	if(value < 0)
	{
		printf("Error when sending data %s\n", &message);
		perror("Couldnt send data");
	}else
	{
		printf("Send %dbytes.\n", value);
	}
	return value;

}

CommandAction * parseReceivedData(char *receivedData)
{
	if(checkCommand(receivedData, "CWD") > -1)
	{
		printf("CWD command\n");
	} else if(checkCommand(receivedData, "PWD") > -1)
	{
		printf("PWD command\n");
	} else
	{
		printf("Invalid command\n");
	}
	return NULL;
}

int checkCommand(char *data, char *command)
{
	if(strstr(data, command) == NULL)
		return -1;
	return 0;
}

void saveFile(char *filename)
{
	int counter;
	FILE *ptr_myfile;
	ptr_myfile=fopen(filename,"wb");
	if (!ptr_myfile)
	{
		printf("Unable to open file!");
		return;
	}
	for ( counter=0; counter < 100; counter++)
	{
		printf("writing %d\n", counter);
		fwrite(&counter, sizeof(int), 1, ptr_myfile);
	}
	fclose(ptr_myfile);

}

int readFile(char *filename, char * buffer, long *bufferSize)
{
	FILE *ptr_myfile;

	ptr_myfile=fopen(filename,"rb");
	if(!ptr_myfile)
	{
		return 0;
	}

	//get flie size
	fseek(ptr_myfile, 0, SEEK_END);
	long filelen = ftell(ptr_myfile);
	rewind(ptr_myfile);


	buffer = (char *)malloc((filelen+1)*sizeof(char));
	fread(buffer, filelen, 1, ptr_myfile);
	*bufferSize = filelen;

	fclose(ptr_myfile);

	return 1;
}

void listFiles()
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir ("root")) != NULL) {
  		/* print all the files and directories within directory */
  		while ((ent = readdir (dir)) != NULL) {
			if(ent->d_type != 4) 
    			printf ("%s, %d\n", ent->d_name, ent->d_type);
  		}
  		closedir (dir);
	} else {
  		/* could not open directory */
  		perror ("Directory error");
	}
}

int isFileExist(char *file)
{
	FILE *fil = fopen(file, "r");
	if(fil == NULL)
	{
		return -1;
	}
	return 1;
}
