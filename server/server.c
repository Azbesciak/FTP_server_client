#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*
	RFC
	https://tools.ietf.org/html/rfc959

	Methods to implement:
		ascii
		binary
		mkdir
		rmdir
		put
		get

*/

#define BUFFER_SIZE 1000
#define MAX_NUMBERS_TO_PARSE 10
void parseReceivedData(char *receivedData, int *numbers, int *numbersSize);
int sumReceivedData(int *numbers, int numbersSize);
int startServer(char * addr, int port);



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
	if(bind(socketNum, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0)
	{
		perror("Binding blad:");
		return -1;
	}
	int time = 1;
	setsockopt(socketNum, SOL_SOCKET, SO_REUSEADDR, (char*)&time, sizeof(time));
	if(listen(socketNum, 100) < 0)
	{
		perror("Blad listen:");
		return -1;
	}
	int sock = sizeof(struct sockaddr);

	int a = 1;

	while(a > 0)
	{
		int recvDs = accept(socketNum, (struct sockaddr *) &remote, &sock);
		if(recvDs< 0)
		{
			perror("Accept error");
		}

		int readBytes = read(recvDs, buff, BUFFER_SIZE);
		char remoteAddr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(remote.sin_addr), remoteAddr, INET_ADDRSTRLEN);
		int remotePort = 2;//ntohs(remote.sin_port);
		printf("Text:%s. from: %s:%d, read bytes: %d\n", buff, remoteAddr, remotePort, readBytes);	 
		int numbers[MAX_NUMBERS_TO_PARSE],numbersSize ;
		parseReceivedData(buff, numbers, &numbersSize);

		int result = sumReceivedData(numbers, numbersSize);

		printf("Sum is %d\n", result);
		sprintf(buff, "%d", result);
		int send = write(recvDs, buff, sizeof(buff) );

		if(send < 0)
		{
			perror("Odpowiedz blad:");
			//a = 0;
			//close(recvDs);
		}else
		{
			printf("bytes sent: %d\n", send);
		}
	}

	close(socketNum);
    return 0;


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
