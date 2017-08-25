#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <iterator>
#include <errno.h>
#include <fcntl.h> /* O_WRONLY, O_CREAT */
#include <unistd.h> /* close, write, read */
#include <string.h>
#include <pthread.h>
#include <unistd.h> //for read and write functions
#include <arpa/inet.h> //for inet_ntop() function
#include <pthread.h>
#include <unistd.h> //for read and write functions
#include <arpa/inet.h> //for inet_ntop() function
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#define pt(a) (cout << "Test case:" << a << endl)
#define BUFFER_SIZE 256
using namespace std;
map <char*, int> ip_map_sessionid;
map <char*, string>ip_map_uname;

struct clientArgs {
    int socket;
};
struct backendArgs {
    char filename[50];
    string userid;
    long filesize;
};
void error(const char *msg){
  perror(msg);
  exit(1);
}

int generatesessionid(char clientip[50],char uname[50]){
	srand (time(NULL)); // generate a seed using the current time
	char *username = (char *)malloc(sizeof(char)*50);
  username = uname; 
	int sessionid = rand(); // generate a random session ID
	cout<<"generated a new session id for client"<<sessionid<<endl;
	ip_map_sessionid.insert(pair <char *, int> (clientip, sessionid)); //insert client ip and sessionid into global map
	// printing map ip_map_sessionid
	ip_map_uname.insert(pair<char*, string> (clientip,username));
  map <char *, int> :: iterator itr;
  cout << "\nThe map ip_map_sessionid is : \n";
  cout << "\tKEY\tELEMENT\n";
  for (itr = ip_map_sessionid.begin(); itr != ip_map_sessionid.end(); ++itr)
  {
        cout  <<  '\t' << itr->first 
              <<  '\t' << itr->second << '\n';
  }
  cout<<endl;
  map <char *, string > :: iterator itr1;
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
	
  pt(2);
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

int connect_to_backend(void)
{
  int clientSocket;

  struct addrinfo hints, // Used to provide hints to getaddrinfo()
                    *res,  // Used to return the list of addrinfo's
                    *p;    // Used to iterate over this list


    /* Host and port */
  const char *host, *port;
  host = "127.0.0.1";
  port = "23300";
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;

   
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(host, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() failed");
        exit(-1);
    }
   for(p = res;p != NULL; p = p->ai_next) 
    {
        
        if ((clientSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            perror("Could not open socket");
            continue;
        }

        
        if (connect(clientSocket, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(clientSocket);
            perror("Could not connect to socket");
            continue;
        }

       
        break;
    }
    
    /* We don't need the linked list anymore. Free it. */
    freeaddrinfo(res);
    return clientSocket;


}
void *send_file_to_backend(void* arguments)
{

  cout<<"inside send_file_to_backend"<<endl;
  struct backendArgs *args = (struct backendArgs *)arguments;

  cout <<args->userid << args->filename <<endl;
  
  int socket = connect_to_backend();
  char toSend[BUFFER_SIZE];
  bzero(toSend,BUFFER_SIZE);
  string fname = string(args->filename);
  string username = string(args->userid);
  //string filesize = string(args->filesize);
  char fname1[50] ;
  //cout<<sizeof(fname);
  fname.copy(fname1,sizeof(fname),0);
  char username1[50];
  username.copy(username1,sizeof(username),0);
  //char filesize1[50];
  //filesize.copy(filesize,sizeof(filesize),0);

  int choice = 1;
  sprintf(toSend,"%d %ld %s %s",choice,args->filesize,username1,fname1);
  //string toSend = "1"+ " " + filesize + " " +username + " " + fname ; 

  cout << "toSend:" << toSend << endl;
  send(socket,toSend,strlen(toSend),0);
  char feedback_from_backend[10]; 
  memset(&feedback_from_backend,0,sizeof(feedback_from_backend));
  int nbytes = recv(socket,feedback_from_backend,sizeof feedback_from_backend,0);
  cout << "feedback char " << feedback_from_backend << endl;
  int received_feedback=0;
  if(nbytes <= 0)
    cout << "server did not receive feedback from backend\n";
  else
    received_feedback = atoi(feedback_from_backend);
  cout << "feedback int" << received_feedback << endl;
  if(received_feedback)
  {
    string fileLocation = args->filename;

  FILE *sendFile = NULL;

  sendFile = fopen(fileLocation.c_str(),"r");

  if(!sendFile)
      fprintf(stderr, "Error fopen ----> %s", strerror(errno));

  int sentData=0;
                    //----buffer chunk to create the file in chunk.
  char chunk[BUFFER_SIZE];
  bzero(chunk,BUFFER_SIZE);
  //memset(&chunk,0,sizeof(chunk));
  int len;
  //-------reading the requested file in chunk.
  while ((len=fread(chunk,1,sizeof chunk, sendFile)) > 0) 
        {  
            len=send(socket,chunk,len,0);
                        
            sentData+=len;

        }
  char response[20];
  bzero(response,20);
  int n = read(socket,response,20);
  cout<<"response from backendserver after sending all chunks at client end is "<<response<<endl;
  cout<<"Total bytes sent to backenserver are = "<<sentData;
  fclose(sendFile);
  close(socket);
  pthread_exit(NULL);
  }
  
  


}
void receive_file_from_client(int sock,char file_name[50],string userId,long filesize){
	cout<<"received filename to upload at server is:"<<file_name<<endl;
	cout<<"inside receive_file_from_client function"<<endl;
// char send_str[BUFFER_SIZE];
 int f; 
 ssize_t sent_bytes, rcvd_bytes, rcvd_file_size;
 int recv_count; 
 char recv_str[BUFFER_SIZE]; 
  recv_count = 0; /* number of recv() calls required to receive the file */
  rcvd_file_size = 0; /* size of received file */
  int counter=0;
 
 if ( (f = open(file_name, O_WRONLY|O_CREAT, 0644)) < 0 )
 {
 error("error creating file");
 //return -1;
 }
 
 cout<<"test after opening file in write mode"<<endl;
 while ((rcvd_bytes = recv(sock, recv_str, BUFFER_SIZE,0)) > 0){
 
 counter++;
 recv_count++;
 rcvd_file_size += rcvd_bytes;

 if (write(f, recv_str, rcvd_bytes) < 0 )
 {
 error("error writing to file");
 //return -1;
 }  
 if(rcvd_file_size==filesize){
 	int n = write(sock,"ack",3);
 	if (n < 0) error("ERROR writing to socket");
 	break;
 }
 
 }
 close(f); /* close file*/


cout<<"Client Received:"<<rcvd_file_size<<" bytes in "<<recv_count<<" recv(s)\n"<<endl;
cout<<"hello"<<endl;
pthread_t handle_backend;
cout<<"hello"<<endl;
struct backendArgs *args;
args = (backendArgs *)malloc(sizeof(struct backendArgs));
cout<<"hello"<<endl;
//cout<<"args->filename= before = "<<args->filename<<endl;

//cout<<"args->userid = before = "<<args->userid<<endl;



args->userid = userId;
cout<<"filesize = "<<filesize;
//char fsize[50];
//sprintf(fsize,"%ld",filesize);
args->filesize = filesize;
//memcpy(args->filesize,fsize,sizeof(args->filesize));
cout<<"args->filesize = "<<args->filesize<<endl;
//args->userid = "userId";
cout<<"args->userid = "<<args->userid<<endl;
cout<<"strlen(file_name)= "<<strlen(file_name)<<endl;
cout<<"sizeof(file_name)= "<<sizeof(file_name)<<endl;
//strncpy(args->filename,file_name,strlen(file_name));
memcpy(args->filename,file_name,sizeof(args->filename));
//args->filename = "file_name";
cout<<"args->filename="<<args->filename<<endl;

cout<<"inserted args in new thread"<<endl;  
if (pthread_create(&handle_backend, NULL, send_file_to_backend,args) != 0) {
        error("Could not create a worker thread");
        free(args);
        
    }
cout<<"hello"<<endl;
pthread_join(handle_backend,NULL);   

 cout<<"going back from receive_file_from_client"<<endl;

}


void session_verify_before_receiving_from_client(int newsockfd, char buffer[BUFFER_SIZE],char clientip[50]){
	cout<<"inside uploaduserfiles()"<<endl;
	char filename[50];
	int n;
	int initial;
	int sid;
	long int filesize;
  sscanf(buffer,"%d %d %s %ld",&initial,&sid,filename,&filesize);
  cout<<"received filename to upload at server is:"<<filename<<endl;
  cout<<"filesize is: "<<filesize<<endl;
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

    cout<<"after for loop"<<endl;
    map <char *, int> :: iterator itr;
  cout << "\nThe map ip_map_sessionid is : \n";
  cout << "\tKEY\tELEMENT\n";
  for (itr = ip_map_sessionid.begin(); itr != ip_map_sessionid.end(); ++itr)
  {
        cout  <<  '\t' << itr->first 
              <<  '\t' << itr->second << '\n';
  }
  cout<<endl;
    map <char *, string > :: iterator itr1;
  cout << "\nThe map ip_map_uname is : \n";
  cout << "\tKEY\tELEMENT\n";
    for (itr1 = ip_map_uname.begin(); itr1 != ip_map_uname.end(); ++itr1)
    {
        cout  <<  '\t' << itr1->first 
              <<  '\t' << itr1->second << '\n';
    }

    cout << endl;

    cout<<"ip_map_sessionid"<<ip_map_sessionid[clientip]<<endl;

    cout<<"ip_map_uname[clientip]"<<ip_map_uname[clientip]<<endl;
    string username = ip_map_uname[clientip];
    cout << "receive_file_from_client():"<< username << endl;
    string fileName = string(username) + "_" + string(filename);
  	cout<<"calling receive_file_from_client"<<endl;


    receive_file_from_client(newsockfd,(char *)fileName.c_str(),username,filesize);
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



void session_verify_before_sending_to_client(int newsockfd, char buffer[BUFFER_SIZE],char clientip[50]){
	cout<<"inside session_verify_before_sending_to_client()"<<endl;
	char filename[50];
	int n;
	int initial;  // 4 sid filename
	int sid;
	long int filesize;
    sscanf(buffer,"%d %d %s",&initial,&sid,filename);
  	cout<<"received filename at server to send to client is:"<<filename<<endl;
  	int sessionactiveflag=checksessionactive(clientip,sid);
  	bzero(buffer,BUFFER_SIZE);

  	//if session active then make a thread to backed to send the requested file to client.
  	if(sessionactiveflag==1){
  		printf("session match found at server\n");

  		// make a call to backend server with message id "2 filename". This function will return the filesize 
  		// which i will send back to client and in turn receive an ack for filesize from client. then we will send file to backend.

  		//long filesize = some function call ()

  		sprintf(buffer,"%d %ld",sessionactiveflag,filesize);
  		n = write(newsockfd,buffer,strlen(buffer));
  		if(n<0){
  			error("Error writing to socket");
  		}
  		// receive filesize response from client
  		bzero(buffer,BUFFER_SIZE);
  		n = read(newsockfd,buffer,BUFFER_SIZE);
  		cout<<"ack from client after sending file is:"<<buffer<<endl;
  		if(!strcmp(buffer,"filesize_received_ack")){
  			//means client successfully received filesize. now start sending file to client.
  			// call some fuinction to send file received from backend server to client.
  			// send_file_to_client_after_receiving_file_from_backend();
  		}
  	} // if close

  	// else means client is not logged in to server. make him login first.
  	else{
  		long filesize = 0;
  		sprintf(buffer,"%d %ld",sessionactiveflag,filesize);
  		n = write(newsockfd,buffer,strlen(buffer));
  		if(n<0){
  			error("Error writing to socket");
  		}
  	} // else close


} //session_verify_before_sending_to_client()  close



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
  		char *garbage=NULL; 
  		int choice;
  		cout<<"buffer received at server is:"<<buffer<<endl;
  		sscanf(buffer,"%d %s",&choice,garbage);
  		cout << choice << endl;
  		switch(choice){
    		case 1:
    		{
      			verifyuserlogin(newsockfd,buffer,ipstr);
      			break;
    		}
			case 2:
    		{
      			pt(1);
      			signupuser(newsockfd,buffer);
      			break;
    		}
			case 3:
			{
				session_verify_before_receiving_from_client(newsockfd,buffer,ipstr);
				break;
			}
			case 4:
			{
				session_verify_before_sending_to_client(newsockfd,buffer,ipstr);
				break;
			}
			case 5:
			{
				clear_session(newsockfd,buffer,ipstr);
				break;
			}
   		}  // switch close
	}	//while close
} //function service_single_client close

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
    cout<<"wa->socket  "<<wa->socket<<endl;

  
  	if (pthread_create(&client_thread, NULL, service_single_client, wa) != 0) 
        {
            perror("Could not create a worker thread");
            free(wa);
            close(newsockfd);
  			//close(sockfd);
            pthread_exit(NULL);
        }
	} // while ends

	close(sockfd);
  pthread_exit(NULL);
	return 0;
}
