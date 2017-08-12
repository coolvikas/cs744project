#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h> //for read and write functions
#include<arpa/inet.h> //for inet_ntop() function
#define BUFFER_SIZE 256

struct clientArgs {
    int socket;
};

void error(const char *msg){
  perror(msg);
  exit(1);
}

void verifyuserlogin(int newsockfd,char buffer[BUFFER_SIZE],char clientip[50]){
	char uname[20],passwd[20];
  int initial;
  sscanf(buffer,"%d %s %s",&initial,uname,passwd);
  printf("Received username passwd from client is:\n" );
  fputs(uname,stdout);
  fputs(" ",stdout);
  fputs(passwd,stdout);
  printf("\n" );
  FILE *fptr;
  char funame[20],fpasswd[20];
  char line[30];
  int flag =0;
  int n;
  fptr = fopen("new.txt","r");
  while(fgets(line,sizeof(line),fptr)!= NULL){
    sscanf(line,"%s%s",funame,fpasswd);
    if(!strcmp(uname,funame)){
      if(!strcmp(passwd,fpasswd)){
      printf("successfully authenticated at server end.\n" );
      flag =1;
      break;
      }
    }

    //fputs(funame,stdout);
    //printf("\t" );
    //fputs(fpasswd,stdout);
    //printf("\n" );

  }
  if(flag ==1 ){
        bzero(buffer,0);
        sprintf(buffer,"1");
        n = write(newsockfd,buffer,strlen(buffer));
        if(n<0){
          error("ERROR writing to socket");
        }
  }

  else{
        bzero(buffer,0);
        sprintf(buffer,"0");
        n = write(newsockfd,buffer,strlen(buffer));
        if(n<0){
          error("ERROR writing to socket");
        }
  }
  fclose(fptr);
  
}


//function to check if user is previously registered or not 

int checkcredentials(char uname[20],char passwd[20]){
	FILE *fptr;
  char funame[20],fpasswd[20];
  char line[30];
  int flag =0;
  int n;
  fptr = fopen("new.txt","r");
  while(fgets(line,sizeof(line),fptr)!= NULL){
    sscanf(line,"%s%s",funame,fpasswd);
    if(!strcmp(uname,funame)){
      if(!strcmp(passwd,fpasswd)){
      flag =1;
      break;
      }
  	}
  }

	fclose(fptr);
	return flag;
}

void signupuser(int newsockfd,char buffer[BUFFER_SIZE]){
	
	char uname[20],passwd[20];
  int initial,n;
  sscanf(buffer,"%d %s %s",&initial,uname,passwd);
  printf("Received username passwd from client is:\n" );
  fputs(uname,stdout);
  fputs(" ",stdout);
  fputs(passwd,stdout);
  printf("\n" );

  FILE *fptr;
  fptr = fopen("new.txt","a");
  char ch[]="\n";
  char empty[] = " ";
  int flag,signupdone;
  flag = checkcredentials(uname,passwd);
  if(flag == 1) //username already exists
  {
    signupdone = 0;
  }
  else{
    signupdone = 1;
    fwrite(uname,strlen(uname),1,fptr); // EACH ELEMENT IS OF SIZE 1 BYTE TO BE WRITTEN AND THERE ARE SIZEOF(BUFFER) ELEMENTS
    fwrite(empty,strlen(empty),1,fptr);
    fwrite(passwd,strlen(passwd),1,fptr);
    fwrite(ch,strlen(ch),1,fptr);

  }
  if(signupdone ==1 ){
     bzero(buffer,0);
     sprintf(buffer,"1");
     n = write(newsockfd,buffer,strlen(buffer));
     if(n<0){
       error("ERROR writing to socket");
  	}
  }

  else{
    bzero(buffer,0);
    sprintf(buffer,"0");
    n = write(newsockfd,buffer,strlen(buffer));
    if(n<0){
      error("ERROR writing to socket");
    }
  }

  fclose(fptr);
 
}

void *service_single_client(void *args){
	struct clientArgs *wa;
	int newsockfd;  

    /* Unpack the arguments */
   wa = (struct clientArgs*) args;
   newsockfd = wa->socket;

	    /* This tells the pthreads library that no other thread is going to
       join() this thread. This means that, once this thread terminates,
       its resources can be safely freed (instead of keeping them around
       so they can be collected by another thread join()-ing this thread) */
  pthread_detach(pthread_self());
  

  fprintf(stderr, "Socket %d connected\n", newsockfd);
	
	char buffer[BUFFER_SIZE];
	bzero(buffer,256);
	int n;

	//get client ip and port from scoket 
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;

	len = sizeof addr;
	getpeername(newsockfd, (struct sockaddr*)&addr, &len);

	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	} 
	else { // AF_INET6
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
    port = ntohs(s->sin6_port);
    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}

	printf("Client IP address: %s\n", ipstr);
	printf("Client port      : %d\n", port);



	while(1){
  n = read(newsockfd,buffer,256);
  if(n<0){
    error("ERROR reading from socket");
  }
  char *garbage; 
  int choice;
  sscanf(buffer,"%d %s",&choice,garbage);

  switch(choice){
    case 1:
    {
      verifyuserlogin(newsockfd,buffer,ipstr);
      break;
    }
		case 2:
    {
      signupuser(newsockfd,buffer);
      break;
    }
		}
	}	//while close

} //function close

int main(int argc, char *argv[]){
	/* The pthread_t type is a struct representing a single thread. */
  pthread_t client_thread;

  int sockfd, newsockfd, portno, x;
  socklen_t clilen;
  
  struct sockaddr_in server_addr, cli_addr;
  int n;
  if(argc<2){
    fprintf(stderr, "ERROR, no port provided\n" );
    exit(1);
  }
  sockfd = socket(AF_INET, SOCK_STREAM,0);
  if(sockfd<0){
    error("ERROR opening socket");
  }
  else{
    printf("Port opened successfully %s \n",(argv[1]) );
  }
  bzero((char *)&server_addr, sizeof(server_addr));
  portno = atoi(argv[1]);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(portno);
  if(bind(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0){
    error("ERROR on binding");
  }
  printf("Listening for incoming connections\n");
  listen(sockfd,5);
	clilen = sizeof(cli_addr);

	while(1){

		/* Call accept(). The thread will block until a client establishes a connection. */
  	newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
  	if(newsockfd < 0){
    	error("ERROR on accept");
  	}

  	struct clientArgs *wa;
  	wa = (clientArgs *)malloc(sizeof(struct clientArgs));
    wa->socket = newsockfd;

  	/* We're now connected to a client. We're going to spawn a "worker thread" to handle
           that connection. That way, the server thread can continue running, accept more connections,
            and spawn more threads to handle them. 
           The worker thread needs to know what socket it must use to communicate with the client,
           so we'll pass the clientSocket as a parameter to the thread. Although we could arguably
           just pass a pointer to clientSocket, it is good practice to use a struct that encapsulates
           the parameters to the thread (even if there is only one parameter). In this case, this is
           done with the workerArgs struct. */
  	if (pthread_create(&client_thread, NULL, service_single_client, wa) != 0) 
        {
            perror("Could not create a worker thread");
            free(wa);
            close(newsockfd);
  					close(sockfd);
            pthread_exit(NULL);
        }

  

  
}
  pthread_exit(NULL);

  /*if(flag == 1){

  } */

  return 0;
}
