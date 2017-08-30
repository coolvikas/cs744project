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
#include <iostream>

#define BUFF_SIZE 256
using namespace std;
const char *port = "23300";

/* We will use this struct to pass parameters to one of the threads */
struct workerArgs
{
    int socket;
};

void error(const char *msg){
  perror(msg);
  exit(1);
}


void addFileInShare(char* dirName,char* fileName,int socket)
{
	string fileLocation = "share.txt";
	
	
	FILE *sendFile = NULL;
	sendFile = fopen(fileLocation.c_str(),"w");
	char ch[] = "\n";
    char empty[] = " ";
    fwrite(fileName,strlen(fileName),1,sendFile); // EACH ELEMENT IS OF SIZE 1 BYTE TO BE WRITTEN AND THERE ARE SIZEOF(BUFFER) ELEMENTS
    fwrite(empty,strlen(empty),1,sendFile);
    fwrite(dirName,strlen(dirName),1,sendFile);
    fwrite(ch,strlen(ch),1,sendFile);
    fclose(sendFile);
    int n = write(socket,"ack",3);
 	if (n < 0) error("ERROR writing to socket");
 			
	

	close(socket);
	pthread_exit(NULL);

}



void sendFile(char* dirName,char* fileName,int socket,long filesize)
{	

	char buffer[BUFF_SIZE];
    bzero(buffer,BUFF_SIZE);
    string fileLocation = string(dirName) + "/" + string(fileName);
	if( access( fileLocation.c_str(), F_OK ) != -1 ) {    //means file exists
    
    
    // to know file size and send it to server
    cout<<"inside sendFile()"<<endl;
    FILE *fptr1 = NULL;
    fptr1 = fopen(fileLocation.c_str(),"r");
    fseek(fptr1,0, SEEK_END);
    long file_len =(unsigned long)ftell(fptr1);
    printf("length of file is%ld\n",file_len);
    fseek(fptr1,0,SEEK_SET);
    fclose(fptr1);
    
    sprintf(buffer,"%ld",file_len);
    int n = write(socket,buffer,BUFF_SIZE);
    char file_size_response[BUFF_SIZE];
    bzero(file_size_response,BUFF_SIZE);
    n = read(socket,file_size_response,BUFF_SIZE);
    cout<<"reponse from server after sending file size is: "<<file_size_response<<endl;

    FILE *sendFile = NULL;


    sendFile = fopen(fileLocation.c_str(),"r");

    if(!sendFile)
        fprintf(stderr, "Error fopen ----> %s", strerror(errno));

    int sentData=0;
                    //----buffer chunk to create the file in chunk.
    char chunk[BUFF_SIZE];
    memset(&chunk,0,sizeof(chunk));
    int len;
                     //-------reading the requested file in chunk.
    while ((len=fread(chunk,1,sizeof chunk, sendFile)) > 0) 
        {  
            len=send(socket,chunk,len,0);
                        
            sentData+=len;
           /* if(sentData == file_len ){
                    

                    int n = write(socket,"ack",3);
                    if (n < 0) error("ERROR writing to socket");
                    break;
                }  */

        }
    cout<<"Total bytes sent to server is = "<<sentData<<endl;   
    char response[20];
  	bzero(response,20);
  	n = read(socket,response,20);
  	cout<<"response from server after receiving all chunks is "<<response<<endl;
    fclose(sendFile);
    close(socket);
}  // if ends

else{     // file does not exist 
	int filefound = 0;
	sprintf(buffer,"%d",filefound);
    int n = write(socket,buffer,BUFF_SIZE);
    if(n<=0){
    	error("sendFile() error writing to socket.");
	}

}

    pthread_exit(NULL);

}


void receiveFile(char* dirName,char* fileName,int socket,long filesize)
{

	int n = write(socket,"backendserver_ready_to_receive",strlen("backendserver_ready_to_receive"));
    struct stat st = {0};

    if (stat(dirName, &st) == -1) 
        mkdir(dirName, 0700);
    if (stat("metadata", &st) == -1) 
        mkdir("metadata", 0700);
    cout << filesize << endl;
    string fileLocation = string(dirName) + "/" + string(fileName);
    FILE *receivedFile = NULL;
    ssize_t len,writtentofile;
    long rcvd_file_size=0;
    receivedFile = fopen(fileLocation.c_str(),"w");

     if (receivedFile == NULL)
        {
                fprintf(stderr, "Failed to open file--> %s\n", strerror(errno));

                exit(EXIT_FAILURE);

        }

    char chunk[BUFF_SIZE];
    memset(&chunk,'\0',sizeof chunk);
   
    //Receiving the file in chunks.

    while ((len = recv(socket, chunk, BUFF_SIZE, 0)) > 0)

        {      
                rcvd_file_size+=len;
                
                if(writtentofile= fwrite(chunk, 1,len, receivedFile)<0){
                	error("cant write to file");
                }
                
                	// if revd file size == filesize it means we received complete file.
                 if(rcvd_file_size==filesize){
                 	cout<<"in if condition rcvd_file_size= "<<rcvd_file_size<<endl<<"filesize="<<filesize<<endl;
                 	cout<<"writing metadata"<<endl;

                 	// write the filename to metadata/uname.txt
                 	char *filename=NULL;
                 	filename = strtok(fileName, "_");
    				filename = strtok(NULL, "_");                 	
                 	string metadata = string("metadata") + "/" + string(dirName);
                 	char* filesizeBuffer = (char *)malloc(sizeof(filesize));
					sprintf(filesizeBuffer,"%ld",filesize);
                 	FILE *metafile = fopen(metadata.c_str(),"a");
    				char ch[] = "\n";
    				char empty[] = " ";
    				fwrite(filename,strlen(filename),1,metafile); // EACH ELEMENT IS OF SIZE 1 BYTE TO BE WRITTEN AND THERE ARE SIZEOF(BUFFER) ELEMENTS
    				fwrite(empty,strlen(empty),1,metafile);
    				fprintf(metafile,"%s",filesizeBuffer);
    				fwrite(ch,strlen(ch),1,metafile);
    				fclose(metafile);
    				cout<<"metadata written"<<endl;

 					int n = write(socket,"ack",3);
 					if (n < 0) error("ERROR writing to socket");
 					break;
 				}
 
    	
        }
        fclose(receivedFile);
        cout<<"rcvd_file_size="<<rcvd_file_size<<endl;
        
        printf("Receiving file %s status %s\n",fileName,strerror(errno));
       close(socket); 
       pthread_exit(NULL);

}


void *service_single_client(void *args) {
    struct workerArgs *wa;
    int socket, nbytes;
    char tosend[100];
    char feedback[10];
    /* Unpack the arguments */
    wa = (struct workerArgs*) args;
    socket = wa->socket;

    /* This tells the pthreads library that no other thread is going to
       join() this thread. This means that, once this thread terminates,
       its resources can be safely freed (instead of keeping them around
       so they can be collected by another thread join()-ing this thread) */
    

    fprintf(stderr, "Socket %d connected\n", socket);

    while(1)
    {
        //sprintf(tosend,"%d Upload OR Download\n", (int) time(NULL));

        //nbytes = send(socket, tosend, strlen(tosend), 0);

        char buff[BUFF_SIZE];
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
    /*    else

         {  int flag=1;
         	bzero(feedback,sizeof(feedback));
            sprintf(feedback,"%d", flag);
            nbytes = send(socket,feedback,strlen(feedback),0);
            if (nbytes != -1)
                cout << "sent feedback " << feedback << endl;
         }                                           */
        cout << "buff:" << buff <<endl;
       int command;
       char fileName[50];
       long filesize;
       char dirName[50] ;
       sscanf(buff,"%d %ld %s %s ",&command,&filesize,dirName,fileName);

       
       cout<<"filesize="<<filesize<<endl;
       
       cout<<"strtok filename = "<<fileName<<endl;
       cout<<"strtok dirName= "<<dirName<<endl;


       
       

       if(command==1)
        receiveFile(dirName,fileName,socket,filesize);

       else if(command==2)
        sendFile(dirName,fileName,socket,filesize);

    	else if(command==3)
    		addFileInShare(dirName,fileName,socket);
       else
        fprintf(stderr, "server did not send proper command\n");

        close(socket);
        free(wa);
        




    }

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
        pthread_detach(worker_thread);
    }

    pthread_exit(NULL);
}




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

   return 0;
}