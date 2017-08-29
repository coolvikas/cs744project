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
#define BUFF_SIZE 256
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
}  // generatesessionid() closed
 
int checksessionactive(char clientip[50],int sessionid){
	int sessionactiveflag=0;
	if (ip_map_sessionid.count(clientip)>0){
		if(ip_map_sessionid[clientip]==sessionid){
			sessionactiveflag=1;
		}
	}
	return sessionactiveflag;
}

void verifyuserlogin(int newsockfd,char buffer[BUFF_SIZE],char clientip[50]){
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
  	}  // else if ends
  
}  // verifyuserlogin() ends


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

void signupuser(int newsockfd,char buffer[BUFF_SIZE]){
	
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

int communicate_with_backend_to_receive(int choice,char* file_name,int size,const char* username)

{
  int sockfd = connect_to_backend();
  cout<<"communicate_with_backend_to_receive() filesize before writing to buffer = "<<size<<endl;
  cout << "In communicate_with_backend :" << " file name :" << file_name << endl;
  char toSend[BUFF_SIZE];
  bzero(toSend,BUFF_SIZE);
  sprintf(toSend,"%d %d %s %s",choice,size,username,file_name);
  cout << "toSend:" << toSend << endl;
  int n = send(sockfd,toSend,strlen(toSend),0);
  if (n<=0){
  	cout<<"communicate_with_backend_to_receive() error wting to socket."<<endl;
  }
  /*
  char feedback_from_backend[BUFF_SIZE]; 
  memset(&feedback_from_backend,0,sizeof(feedback_from_backend));
  int nbytes = recv(sockfd,feedback_from_backend,sizeof feedback_from_backend,0);
  cout << "feedback from backend:" << feedback_from_backend << endl;
  int received_feedback=0;
  if(nbytes <= 0)
    cout << "server did not receive feedback from backend\n";
  else{
    received_feedback = atoi(feedback_from_backend);
    return sockfd;
  }      */
  return sockfd;      
}


int communicate_with_backend_to_send(int choice,char* file_name,int size,const char* username)

{
  int sockfd = connect_to_backend();

  cout << "In communicate_with_backend :" << " file name :" << file_name << endl;
  char toSend[BUFF_SIZE];
  bzero(toSend,BUFF_SIZE);
  sprintf(toSend,"%d %d %s %s",choice,size,username,file_name);
  cout << "toSend:" << toSend << endl;
  send(sockfd,toSend,strlen(toSend),0);
  
  char feedback_from_backend[BUFF_SIZE]; 
  memset(&feedback_from_backend,0,sizeof(feedback_from_backend));
  int nbytes = recv(sockfd,feedback_from_backend,sizeof feedback_from_backend,0);
  cout << "feedback from backend:" << feedback_from_backend << endl;
  int received_feedback=0;
  if(nbytes <= 0)
    cout << "server did not receive feedback from backend\n";
  else{
    received_feedback = atoi(feedback_from_backend);
    return sockfd;
  }            
}






void *send_file_to_backend(void* arguments){

	cout<<"inside send_file_to_backend\n"<<endl;
  	struct backendArgs *args = (struct backendArgs *)arguments;
	
  
  
  char toSend[BUFF_SIZE];
  bzero(toSend,BUFF_SIZE);
  char file_name[50];
  bzero(file_name,sizeof file_name);

  memcpy(file_name,args->filename,sizeof(args->filename));
  string username = string(args->userid);
  
  
  int choice = 1;
  int filesize = args->filesize;

  
  cout << "send_file_to_backend: file name " << file_name << endl;
  
  int socket=communicate_with_backend_to_send(choice,file_name,filesize,username.c_str());
 
  string fileLocation = args->filename;
  cout << "filelocation: " << fileLocation << endl;
  FILE *sendFile = NULL;

  sendFile = fopen(fileLocation.c_str(),"r");

  if(!sendFile)
      fprintf(stderr, "Error fopen ----> %s", strerror(errno));

  int sentData=0;
                    //----buffer chunk to create the file in chunk.
  char chunk[BUFF_SIZE];
  bzero(chunk,BUFF_SIZE);
  //memset(&chunk,0,sizeof(chunk));
  int len;
  
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

void *receive_file_from_backend(void* arguments)
{

  cout << "In receive file from backend" << endl;
  struct backendArgs *args = (struct backendArgs *)arguments;
  
  char toSend[BUFF_SIZE];
  bzero(toSend,BUFF_SIZE);
  char file_name[50];
  bzero(file_name,sizeof(file_name));

  memcpy(file_name,args->filename,sizeof(file_name));
  string username = string(args->userid);
  int n;
 
  int choice = 2;

  // this file size is not used 

  int filesize = args->filesize;
  cout<<"receive_file_from_backend() filesize passed = "<<filesize<<endl;
  
  int socket=communicate_with_backend_to_receive(choice,file_name,filesize,username.c_str());
  
  string fileLocation = args->filename;

  long received_file_size_from_backend;
  char buffer[BUFF_SIZE];
  bzero(buffer,BUFF_SIZE);
  n = read(socket,buffer,BUFF_SIZE);
  sscanf(buffer,"%ld",&received_file_size_from_backend);
  cout<<"received_file_size_from_backend="<<received_file_size_from_backend<<endl;
  n = write(socket,"filesize_received_ack",strlen("filesize_received_ack"));
  FILE *receivedFile = NULL;

  receivedFile = fopen(fileLocation.c_str(),"w");
    if(!receivedFile)
      fprintf(stderr, "Error fopen ----> %s", strerror(errno));

  int receivedData=0;
                    //----buffer chunk to create the file in chunk.
  char chunk[BUFF_SIZE];
  bzero(chunk,BUFF_SIZE);
  //memset(&chunk,0,sizeof(chunk));
  int len;

   while ((len = recv(socket, chunk, BUFF_SIZE, 0)) > 0)

        {       
                receivedData+=len;
              
                if(n= fwrite(chunk, 1,len, receivedFile)<0){
                  error("cant write to file");
                }
                
                 if(receivedData==received_file_size_from_backend)
                  {
                    cout << "file received completely\n";
                    
                    int n = write(socket,"ack",3);
                    if (n < 0) error("ERROR writing to socket");
                    break;
                  }

                
        
 
      
        }
  /*char response[20];
  bzero(response,20);
  n = read(socket,response,20);
  cout<<"response from backendserver after sending all chunks to server end is "<<response<<endl;  */
  cout<<"Total bytes received to server is = "<<receivedData<<endl;
  fclose(receivedFile);
  close(socket);
  pthread_exit(NULL);

  

}

void handle_backend(char file_name[50],string userId,long filesize,int choice)
{

  cout << "In handle_backend() file name = " << file_name << endl;
  pthread_t handle_backend;
  
  struct backendArgs *args;
  args = (backendArgs *)malloc(sizeof(struct backendArgs));
  
  args->userid = userId;
  
  args->filesize = filesize;
  
  memcpy(args->filename,file_name,sizeof(args->filename));
  
  cout << "args filename: " << args->filename << endl;

  cout << "choice: " << choice << endl;
  if(choice == 1)
    {
      if (pthread_create(&handle_backend, NULL, send_file_to_backend,args) != 0)
      {
        error("Could not create a worker thread");
        free(args);
      }
      pthread_join(handle_backend,NULL);
    }
  else if(choice == 2)
     { cout << "Inside else if" << endl; 

      if (pthread_create(&handle_backend, NULL, receive_file_from_backend,args) != 0)
      {
        error("Could not create a worker thread");
        free(args);
      }
      pthread_join(handle_backend,NULL);
    }
    free(args);
     

}

void send_file(int socket,char file_name[50],string userId)
{

    cout << "In send_file\n";
    string fileLocation = string(file_name);

    FILE *sendFile = NULL;

    sendFile = fopen(fileLocation.c_str(),"r");

    if(!sendFile)
        fprintf(stderr, "Error fopen ----> %s", strerror(errno));

    int sentData=0;
                    //----buffer chunk to create the file in chunk.
    char chunk[BUFF_SIZE];
    memset(&chunk,0,sizeof(chunk));
    int len=0;
                     //-------reading the requested file in chunk.
    while ((len=fread(chunk,1,sizeof chunk, sendFile)) > 0) 
        {  
        	cout<<".";
            len=send(socket,chunk,len,0);
                        
            sentData+=len;
           /* if(sentData == filesize){
                    break;
                }  */

        }
    cout<<"Server sent "<<sentData<<" bytes to client successfully !!"<<endl;
    fclose(sendFile);
   
  
}

int receive_file(int socket,char file_name[50],string userId,long filesize)
{
	
 	  string fileLocation = string(file_name);
    
    cout << "file name: " << file_name << "userId: " << userId << "fileLocation:" << fileLocation << endl;

 	  FILE *receivedFile = NULL;
    
    receivedFile = fopen(fileLocation.c_str(),"w");
    if(!receivedFile)
        fprintf(stderr, "Error fopen ----> %s", strerror(errno));
    
    int receivedData=0;
                    //----buffer chunk to create the file in chunk.
    char chunk[BUFF_SIZE];
    memset(&chunk,0,sizeof(chunk));
    int len;
 

     while ((len = recv(socket, chunk, BUFF_SIZE, 0)) > 0)

        {       
                receivedData+=len;
                
                if(int n = fwrite(chunk, 1,len, receivedFile)<0){
                  error("cant write to file");
                }
                
                 if(receivedData==filesize)
                  {
                  

                      int n = write(socket,"ack",3);
                      if (n < 0) error("ERROR writing to socket");
                      break;
                  }
 
      
        }


      fclose(receivedFile);
      return 1;
	
}  


void receive_from_client(int newsockfd, char buffer[BUFF_SIZE],char clientip[50]){
	
	char filename[50];
	int n;
	int initial;
	int sid;
	long int filesize;
    sscanf(buffer,"%d %d %s %ld",&initial,&sid,filename,&filesize);
    cout << "In receive_from_client: filename " << filename << endl;	
  	int sessionactiveflag=checksessionactive(clientip,sid);
  	bzero(buffer,BUFF_SIZE);
    if(sessionactiveflag==1)
    {
  		printf("session match found at server\n");
  		sprintf(buffer,"%d",sessionactiveflag);
  		n = write(newsockfd,buffer,strlen(buffer));

  		if(n<0)
        error("Error writing to socket");
  	
    string username = ip_map_uname[clientip];
    
    string fileName = string(username) + "_" + string(filename);
  


    int status = receive_file(newsockfd,(char *)fileName.c_str(),username,filesize);
    
    if(status)
      handle_backend((char *)fileName.c_str(),username,filesize,1);

    else
      printf("could not receive file from client\n");

  	
  }

  // else notify client to login first
  else
  {
  	printf("session match not found at server\n");
  	sprintf(buffer,"%d",sessionactiveflag);
  	n = write(newsockfd,buffer,strlen(buffer));
  	if(n<0){
  		error("Error writing to socket");
  	}

  }
  

}


void send_to_client(int newsockfd, char buffer[BUFF_SIZE],char clientip[50])
{
  
  cout<<"inside send to client"<<endl;
  char filename[50];
  bzero(filename,sizeof(filename));
  int n;
  int initial;  // 4 sid filename
  int sid;
  long int filesize=0;
  sscanf(buffer,"%d %d %s",&initial,&sid,filename);
  cout<<"received filename at server to send to client is:"<<filename<<endl;
  int sessionactiveflag=checksessionactive(clientip,sid);
  bzero(buffer,BUFF_SIZE);
  string username = ip_map_uname[clientip];
  cout << "Sending file to client:"<< username << endl;
  string fileName = string(username) + "_" + string(filename);
    
  if(sessionactiveflag==1){
    printf("session match found at server\n");

    
    int is_file_available = 1;
    //is_file_available = verify_from_user_filestat(filename,username);

    if (is_file_available)
      {

        handle_backend((char *)fileName.c_str(),username,filesize,2);
        FILE *fptr1 = NULL;
    	fptr1 = fopen(fileName.c_str(),"r");
    	fseek(fptr1,0, SEEK_END);
    	long file_len =(unsigned long)ftell(fptr1);
    	printf("length of file is%ld\n",file_len);
    	fseek(fptr1,0,SEEK_SET);
    	fclose(fptr1);

        sprintf(buffer,"%d %ld",sessionactiveflag,file_len);
        n = write(newsockfd,buffer,strlen(buffer));
        if(n<0){
            error("Error writing to socket");
              }

        bzero(buffer,BUFF_SIZE);
        n = read(newsockfd,buffer,BUFF_SIZE);
        cout<<"Response from client after sending file size is:"<<buffer<<endl;
      	send_file(newsockfd,(char *)fileName.c_str(),username);
      	cout << "sent complete file to client\n";
      	bzero(buffer,BUFF_SIZE);
      	n = read(newsockfd,buffer,3);
      	cout << "message from client : " << buffer << endl;
      
        
      }
    

      
    } 
    
    else{
      long filesize = 0;
      sprintf(buffer,"%d %ld",sessionactiveflag,filesize);
      n = write(newsockfd,buffer,strlen(buffer));
      if(n<0){
        error("Error writing to socket");
      }
    } // else close


} 


// clear_session 
void clear_session(int newsockfd, char buff[BUFF_SIZE], char clientip[50]){
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
	char buffer[BUFF_SIZE];
	bzero(buffer,BUFF_SIZE);
	sprintf(buffer,"%d",session_cleared);
	cout<<"logout():value before witing buffer is:"<<buffer;
	int n = write(newsockfd,buffer,strlen(buffer));
    if(n<0){
  		error("Error writing to socket");
    }
}

//this function gets the list of files available with backend for a particular client and stores them temporarily here.
void get_filesystem_from_backend(int newsockfd,char buffer[BUFF_SIZE],char clientip[50]){
	int choice,sessionid;
	cout<<"inside get_filesystem_from_backend()"<<endl;
	sscanf(buffer,"%d %d",&choice,&sessionid);
	cout<<"sessionid = "<<sessionid<<endl;
	if(checksessionactive(clientip,sessionid)){  // means session is active
		cout<<"Session match found at server."<<endl;
		char fname[50];
		string dirname = "metadata";
		int choice =1;
		memcpy(fname,ip_map_uname[clientip].c_str(),sizeof(fname));
		if( access( fname, F_OK ) != -1 ) {  //access return value is 0 if the access is permitted, and -1 otherwise.
    			// file exists locally send to client and display
			FILE *fptr1 = NULL;
    		fptr1 = fopen(fname,"r");
    		fseek(fptr1,0, SEEK_END);
    		long file_len =(unsigned long)ftell(fptr1);
    		printf("length of file is%ld\n",file_len);
    		fseek(fptr1,0,SEEK_SET);
    		fclose(fptr1);

    		char buffer[BUFF_SIZE];
    		bzero(buffer,BUFF_SIZE);
    		sprintf(buffer,"%d %ld",choice,file_len);
    		int n = write(newsockfd,buffer,strlen(buffer));
        	if(n<0){
            	error("Error writing to socket");
            }

        	bzero(buffer,BUFF_SIZE);
        	n = read(newsockfd,buffer,BUFF_SIZE);
        	cout<<"Response from client after sending file size is:"<<buffer<<endl;

			send_file(newsockfd,fname,fname);
			cout << "sent complete file to client\n";
      		bzero(buffer,BUFF_SIZE);
      		n = read(newsockfd,buffer,3);
      		cout << "message from client : " << buffer << endl;
      		return;


		} 		
		else {
			cout<<"Files does not exists calling handle_backend()"<<endl;
    		// file doesn't exist  make connection to backend to receive file
			handle_backend(fname,dirname,0,2);
			cout<<"in get_filesystem_from_backend after handle_backend() call completed"<<endl;
			char buffer[BUFF_SIZE];
			bzero(buffer,BUFF_SIZE);
			FILE *fptr1 = NULL;
    		fptr1 = fopen(ip_map_uname[clientip].c_str(),"r");
    		fseek(fptr1,0, SEEK_END);
    		long file_len =(unsigned long)ftell(fptr1);
    		printf("length of file is%ld\n",file_len);
    		fseek(fptr1,0,SEEK_SET);
    		fclose(fptr1);

        	sprintf(buffer,"%d %ld",choice,file_len);
        	int n = write(newsockfd,buffer,strlen(buffer));
        	if(n<0){
            	error("Error writing to socket");
              }

        	bzero(buffer,BUFF_SIZE);
        	n = read(newsockfd,buffer,sizeof (buffer));
        	cout<<"Response from client after sending file size is:"<<buffer<<endl;
      		send_file(newsockfd,fname,ip_map_uname[clientip].c_str());
      		cout << "sent complete file to client\n";
      		bzero(buffer,BUFF_SIZE);
      		n = read(newsockfd,buffer,3);
      		cout << "message from client : " << buffer << endl;
      		return;

		}


	}  // if closes
	else{
		int response = 0;
		char *responsebuffer = NULL;
		char buffer[BUFF_SIZE];
		bzero(buffer,BUFF_SIZE);
		sprintf(buffer,"%d %s",response,responsebuffer);
		int n =write(newsockfd,buffer,strlen(buffer));
		if(n<0){
			cout<<"get_filesystem_from_backend():Error writing to socket"<<endl;
		}

	}  // else closes
	return;

	
} // get_filesystem() closed









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
		char buffer[BUFF_SIZE];
		bzero(buffer,BUFF_SIZE);
		int n;
  		n = read(newsockfd,buffer,256);
  		if(n<0){
    		error("ERROR reading from socket");
  		}
  		char *garbage=NULL; 
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
      			pt(1);
      			signupuser(newsockfd,buffer);
      			break;
    		}
			case 3:
			{
				receive_from_client(newsockfd,buffer,ipstr);
				break;
			}
			case 4:
			{
				get_filesystem_from_backend(newsockfd,buffer,ipstr);
				break;
			}
			case 5:
			{
				send_to_client(newsockfd,buffer,ipstr);
				break;
			}
			case 10:
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
