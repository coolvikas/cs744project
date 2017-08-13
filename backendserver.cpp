#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

using namespace std;
const char *port = "23300";

/* We will use this struct to pass parameters to one of the threads */
struct workerArgs
{
    int socket;
};

void *accept_clients(void *args);
void *service_single_client(void *args);

int main(int argc, char *argv[])
{
    /* The pthread_t type is a struct representing a single thread. */
    pthread_t server_thread;

    
    if (pthread_create(&server_thread, NULL, accept_clients, NULL) < 0)
    {
        perror("Could not create server thread");
        exit(-1);
    }

    pthread_join(server_thread, NULL);

    pthread_exit(NULL);
}



void *accept_clients(void *args)
{
    int serverSocket;
    int clientSocket;
    pthread_t worker_thread;
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage *clientAddr;
    socklen_t sinSize = sizeof(struct sockaddr_storage);
    struct workerArgs *wa;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Return my address, so I can bind() to it

    /* Note how we call getaddrinfo with the host parameter set to NULL */
    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() failed");
        pthread_exit(NULL);
    }

    for(p = res;p != NULL; p = p->ai_next) 
    {
        if ((serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            perror("Could not open socket");
            continue;
        }

        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("Socket setsockopt() failed");
            close(serverSocket);
            continue;
        }

        if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("Socket bind() failed");
            close(serverSocket);
            continue;
        }

        if (listen(serverSocket, 5) == -1)
        {
            perror("Socket listen() failed");
            close(serverSocket);
            continue;
        }

        break;
    }
    
    freeaddrinfo(res);

    if (p == NULL)
    {
        fprintf(stderr, "Could not find a socket to bind to.\n");
        pthread_exit(NULL);
    }

    /* Loop and wait for connections */
    while (1)
    {
        /* Call accept(). The thread will block until a client establishes a connection. */

        clientAddr = (struct sockaddr_storage *)malloc(sinSize);
        if ((clientSocket = accept(serverSocket, (struct sockaddr *) clientAddr, &sinSize)) == -1) 
        {
            /* If this particular connection fails, no need to kill the entire thread. */
            free(clientAddr);
            perror("Could not accept() connection");
            continue;
        }

        /* We're now connected to a client. We're going to spawn a "worker thread" to handle
           that connection. That way, the server thread can continue running, accept more connections,
            and spawn more threads to handle them. 
           The worker thread needs to know what socket it must use to communicate with the client,
           so we'll pass the clientSocket as a parameter to the thread. Although we could arguably
           just pass a pointer to clientSocket, it is good practice to use a struct that encapsulates
           the parameters to the thread (even if there is only one parameter). In this case, this is
           sone with the workerArgs struct. */
        wa = (struct workerArgs *)malloc(sizeof(struct workerArgs));
        wa->socket = clientSocket;

        if (pthread_create(&worker_thread, NULL, service_single_client, wa) != 0) 
        {
            perror("Could not create a worker thread");
            free(clientAddr);
            free(wa);
            close(clientSocket);
            close(serverSocket);
            pthread_exit(NULL);
        }
    }

    pthread_exit(NULL);
}


/* This is the function that is run by the "worker thread".
   It is in charge of "handling" an individual connection and, in this case
   all it will do is send a message every five seconds until the connection
   is closed.
   See oneshot-single.c and client.c for more documentation on how the socket
   code works.
 */
void sendFile(char* dirName,char* fileName,int socket)
{
    string fileLocation = string(dirName) + "/" + string(fileName);

    FILE *sendFile = NULL;

    sendFile = fopen(fileLocation.c_str(),"r");

    if(!sendFile)
        fprintf(stderr, "Error fopen ----> %s", strerror(errno));

    int sentData=0;
                    //----buffer chunk to create the file in chunk.
    char chunk[30];
    memset(&chunk,0,sizeof(chunk));
    int len;
                     //-------reading the requested file in chunk.
    while ((len=fread(chunk,1,sizeof chunk, sendFile)) > 0) 
        {  
            len=send(socket,chunk,len,0);
                        
            sentData+=len;

        }
    fclose(sendFile);
    close(socket);
    pthread_exit(NULL);

}


void receiveFile(char* dirName,char* fileName,int socket)
{
    struct stat st = {0};

    if (stat(dirName, &st) == -1) 
        mkdir(dirName, 0700);
    
    string fileLocation = string(dirName) + "/" + string(fileName);

    FILE *receivedFile;
    int remainData,fileSize;
    ssize_t len;
    receivedFile = fopen(fileLocation.c_str(),"w");

     if (receivedFile == NULL)
        {
                fprintf(stderr, "Failed to open file--> %s\n", strerror(errno));

                exit(EXIT_FAILURE);

        }

    char chunk[30];
    memset(&chunk,'\0',sizeof chunk);
   
    //Receiving the file in chunks.

    while ((len = recv(socket, chunk, sizeof(chunk), 0)) > 0)
        {       
                
                //cout<<chunk<<" "<<len<<endl;
                fwrite(chunk, 1,len, receivedFile);
    
        }
        fclose(receivedFile);
        
       // printf("Receiving file %s status %s\n",fileName.c_str(),strerror(errno));
       close(socket); 
       pthread_exit(NULL);

}
void *service_single_client(void *args) {
    struct workerArgs *wa;
    int socket, nbytes;
    char tosend[100];

    /* Unpack the arguments */
    wa = (struct workerArgs*) args;
    socket = wa->socket;

    /* This tells the pthreads library that no other thread is going to
       join() this thread. This means that, once this thread terminates,
       its resources can be safely freed (instead of keeping them around
       so they can be collected by another thread join()-ing this thread) */
    pthread_detach(pthread_self());

    fprintf(stderr, "Socket %d connected\n", socket);

    while(1)
    {
        sprintf(tosend,"%d Upload OR Download\n", (int) time(NULL));

        nbytes = send(socket, tosend, strlen(tosend), 0);

        char buff[100];
        memset(&buff,0,sizeof(buff));
        
        if ((nbytes = recv(socket, buff, sizeof buff, 0)) <= 0)
            {
                        
            if (nbytes == 0) {
                // connection closed
                printf("connection %d closed\n", socket);
                close(socket);
                free(wa);
                pthread_exit(NULL);

                        }
            else {
                perror("recv");
                 }
            }
       char *command = strtok (buff," ");
       
       char *dirName = strtok (NULL," ");

       char *fileName = strtok (NULL," ");

       if(!strcmp(command,"1"))
        receiveFile(dirName,fileName,socket);

       else if(!strcmp(command,"2"))
        sendFile(dirName,fileName,socket);

       else
        fprintf(stderr, "server did not send proper command\n");

        close(socket);
        free(wa);
        pthread_exit(NULL);





        sleep(5);
    }

    pthread_exit(NULL);
}
