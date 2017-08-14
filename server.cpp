#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <map>
#include <iterator>
#include <fcntl.h> /* O_WRONLY, O_CREAT */
#include <unistd.h> /* close, write, read */
#include<string.h>
#include<pthread.h>
#include<unistd.h> //for read and write functions
#include<arpa/inet.h> //for inet_ntop() function
#define BUFFER_SIZE 256
using namespace std;

map <char*, int> ip_map_sessionid;
map <char*, char*>ip_map_uname;

struct clientArgs {
    int socket;
};

void error(const char *msg){
  perror(msg);
  exit(1);
}

int generatesessionid(char clientip[50],char uname[50]){
	srand (time(NULL)); // generate a seed using the current time
	
	int sessionid = rand(); // generate a random session ID
	cout<<"generated a new session id for client"<<sessionid<<endl;
	ip_map_sessionid.insert(pair <char *, int> (clientip, sessionid)); //insert client ip and sessionid into global map
	// printing map ip_map_sessionid
	ip_map_uname.insert(pair<char*, char*> (clientip,uname));
  map <char *, int> :: iterator itr;
  cout << "\nThe map ip_map_sessionid is : \n";
  cout << "\tKEY\tELEMENT\n";
  for (itr = ip_map_sessionid.begin(); itr != ip_map_sessionid.end(); ++itr)
  {
        cout  <<  '\t' << itr->first 
              <<  '\t' << itr->second << '\n';
  }
  cout<<endl;
  map <char *, char* > :: iterator itr1;
  cout << "\nThe map ip_map_uname is : \n";
  cout << "\tKEY\tELEMENT\n";
  for (itr1 = ip_map_uname.begin(); itr1 != ip_map_uname.end(); ++itr1)
  {
        cout  <<  '\t' << itr1->first 
              <<  '\t' << itr1->second << '\n';
  }

  cout << endl;
  return sessionid;
}

int checksessionactive(char clientip[50],int sessionid){
	int sessionactiveflag=0;
	if (ip_map_sessionid.count(clientip)>0){
		if(ip_map_sessionid[clientip]==sessionid){
			sessionactiveflag=1;
		}
	}
	return sessionactiveflag;
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
  cout<<"opened file to find client name"<<endl;
  while(fgets(line,sizeof(line),fptr)!= NULL){
    sscanf(line,"%s%s",funame,fpasswd);
    if(!strcmp(uname,funame)){
      if(!strcmp(passwd,fpasswd)){
      printf("successfully authenticated at server end.\n" );
      flag =1;
      break;
      }
    }

  } //while ends
  fclose(fptr);
  cout<<"client ip count is:"<<ip_map_sessionid.count(clientip)<<endl;
  if (ip_map_sessionid.count(clientip)>0){
  	flag = 2;
  }
  if(flag ==1 ){

  			cout<<"Generating a session id"<<endl;
  			int sessionid = generatesessionid(clientip,uname);
        bzero(buffer,0);
        sprintf(buffer,"%d %d",flag,sessionid);
        n = write(newsockfd,buffer,strlen(buffer));
        if(n<0){
          error("ERROR writing to socket");
        }
  }

  else if(flag == 0){
        bzero(buffer,0);
        int sessionid = 0;
        sprintf(buffer,"%d %d",flag,sessionid);
        cout<<"value in buffer before witing to client is"<<buffer<<endl;
        n = write(newsockfd,buffer,strlen(buffer));
        if(n<0){
          error("ERROR writing to socket");
        }
  }
  else if(flag == 2){
  	bzero(buffer,0);
  	int sid = ip_map_sessionid[clientip];
  	sprintf(buffer,"%d %d",flag,sid);
  	 n = write(newsockfd,buffer,strlen(buffer));
     if(n<0){
       error("ERROR writing to socket");
     }
  }
  
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

void receive_file_from_client(int sock, char file_name[50]){
	cout<<"received filename to upload at server is:"<<file_name<<endl;
	cout<<"inside receive_file_from_client function"<<endl;
 char send_str[BUFFER_SIZE]; /* message to be sent to server*/
 int f; /* file handle for receiving file*/
 ssize_t sent_bytes, rcvd_bytes, rcvd_file_size;
 int recv_count; /* count of recv() calls*/
 char recv_str[BUFFER_SIZE]; /* buffer to hold received data */
 
 	// here buffer receives number of partitions of file from client.
  char buffer1[BUFFER_SIZE];
  bzero(buffer1,BUFFER_SIZE);
  int n;
  n = read(sock,buffer1,BUFFER_SIZE);
  if(n<0){
      error("ERROR reading from socket");
  }
  long partitions;
  sscanf(buffer1,"%ld",&partitions);

 /* attempt to create file to save received data. 0644 = rw-r--r-- */ 
 if ( (f = open(file_name, O_WRONLY|O_CREAT, 0644)) < 0 )
 {
 error("error creating file");
 //return -1;
 }
 cout<<"test after opening file in write mode"<<endl;
 recv_count = 0; /* number of recv() calls required to receive the file */
 rcvd_file_size = 0; /* size of received file */
int counter=1;
 /* continue receiving until ? (data or close) */
 while ( ((rcvd_bytes = recv(sock, recv_str, BUFFER_SIZE,0)) > 0)&& counter<=partitions )
 {
 	counter++;
 	//cout<<"inside while loop"<<counter<<endl;
 recv_count++;
 rcvd_file_size += rcvd_bytes;

 if (write(f, recv_str, rcvd_bytes) < 0 )
 {
 error("error writing to file");
 //return -1;
 }
 }
 close(f); /* close file*/
 cout<<"Client Received:"<<rcvd_file_size<<" bytes in "<<recv_count<<" recv(s)\n"<<endl;
 //return rcvd_file_size;
}

void printhello(){
	cout<<"inside hello function";
}

void session_verify_before_receiving_from_client(int newsockfd, char buffer[BUFFER_SIZE],char clientip[50]){
	cout<<"inside uploaduserfiles()"<<endl;
	char filename[50];
	int n;
	int initial;
	int sid;
  sscanf(buffer,"%d %d %s",&initial,&sid,filename);
  cout<<"received filename to upload at server is:"<<filename;
  int sessionactiveflag=checksessionactive(clientip,sid);
  bzero(buffer,BUFFER_SIZE);

  //if session active then call receive_file_from_client
  if(sessionactiveflag==1){
  	printf("session match found at server\n");
  	sprintf(buffer,"%d",sessionactiveflag);
  	n = write(newsockfd,buffer,strlen(buffer));
  	if(n<0){
  		error("Error writing to socket");
  	}
  	cout<<"calling receive_file_from_client"<<endl;
  	printhello();
  	receive_file_from_client(newsockfd,filename);
  	cout<<"completed calling receive_file_from_client"<<endl;
  }

  // else notify client to login first
  else{
  	printf("session match not found at server\n");
  	sprintf(buffer,"%d",sessionactiveflag);
  	n = write(newsockfd,buffer,strlen(buffer));
  	if(n<0){
  		error("Error writing to socket");
  	}

  }
  cout<<"completed calling session_verify_before_receiving_from_client"<<endl;

}

void clear_session(int newsockfd, char buff[BUFFER_SIZE], char clientip[50]){
	int session_cleared=0;
	int client_sessionid,choice;
	sscanf(buff,"%d %d",&choice,&client_sessionid);
	if (ip_map_sessionid.count(clientip)>0){
		if(ip_map_sessionid[clientip]==client_sessionid){
			session_cleared=1;
	 		ip_map_uname.erase (clientip);
	 		ip_map_sessionid.erase (clientip);
		}
	}
	char buffer[BUFFER_SIZE];
	bzero(buffer,BUFFER_SIZE);
	sprintf(buffer,"%d",session_cleared);
	cout<<"logout():value before witing buffer is:"<<buffer;
	int n = write(newsockfd,buffer,strlen(buffer));
  if(n<0){
  		error("Error writing to socket");
  }
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
	char buffer[BUFFER_SIZE];
	bzero(buffer,BUFFER_SIZE);
	int n;
  n = read(newsockfd,buffer,256);
  if(n<0){
    error("ERROR reading from socket");
  }
  char *garbage; 
  int choice;
  cout<<"buffer received at server is:"<<buffer<<endl;
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
		
		case 3:
		{
			session_verify_before_receiving_from_client(newsockfd,buffer,ipstr);
			break;
		}
		case 5:
		{
			clear_session(newsockfd,buffer,ipstr);
			break;
		}
	}  // switch close
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
	} // while ends


  pthread_exit(NULL);
	return 0;
}
