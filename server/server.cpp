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
#include <fstream>
// #define pt(a) (cout << "Test case:" << a << endl)
#define BUFF_SIZE 256
using namespace std;
pthread_mutex_t sharedfilelock;
const char *shared_file = "share.txt";
const char *auth_file = "new.txt";
map <char*, int> ip_map_sessionid;
map <char*, string>ip_map_uname;
char *backend_ip;
char *backend_port;
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
void handle_backend(char [],string,long ,int );
void get_client_metadata_file(char filename[30]){
	// this function will get clients metadta file from backend server if the client logs in successfully and session ID is generated
	string dirname = "metadata";
	int choice =1;
	handle_backend(filename,dirname,0,2);
}

int generatesessionid(char clientip[50],char uname[50]){
	srand (time(NULL)); // generate a seed using the current time
	char *username = (char *)malloc(sizeof(char)*50);
  	username = uname; 
	int sessionid = rand(); // generate a random session ID
	cout<<"generated a new session id for client"<<sessionid<<endl;
	ip_map_sessionid.insert(pair <char *, int> (clientip, sessionid)); //insert client ip and sessionid into global map
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
  	get_client_metadata_file(uname);
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
  	FILE *fptr;
  	char funame[20],fpasswd[20];
  	char line[30];
  	int flag =0;
  	int n;
  	fptr = fopen(auth_file,"r");
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
  	if (ip_map_sessionid.count(clientip)>0){
  		flag = 2;
 	}

  	if(flag ==1 ){
		cout<<"Generating a session id"<<endl;
  		int sessionid = generatesessionid(clientip,uname);
  		cout<<"Generated session ID for client is = "<<sessionid<<endl;
        bzero(buffer,0);
        sprintf(buffer,"%d %d",flag,sessionid);
        n = write(newsockfd,buffer,strlen(buffer));
        if(n<0){
          error("ERROR writing to socket");
        }
  	}

  	else if(flag == 0){
  		cout<<"Wrong credentials received. Try again to login !!"<<endl;
        bzero(buffer,0);
        int sessionid = 0;
        sprintf(buffer,"%d %d",flag,sessionid);
        //cout<<"value in buffer before witing to client is"<<buffer<<endl;
        n = write(newsockfd,buffer,strlen(buffer));
        if(n<0){
          error("ERROR writing to socket");
        }
  	}
  	else if(flag == 2){
  		cout<<"Client is already logged in !!"<<endl;
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

int checkcredentials(char uname[20],char passwd[20])
{
	FILE *fptr;
  	char funame[20],fpasswd[20];
  	char line[30];
  	int flag =0;
  	int n;
  	fptr = fopen(auth_file,"r");
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

void signupuser(int newsockfd,char buff[BUFF_SIZE],char clientip[50])
{
	int signupdone;
	int n;
  	map <char*, int>::iterator it;
  	//cout << "In signup" << endl;
  	it = ip_map_sessionid.find(clientip);
  	char buffer[BUFF_SIZE];
  
	if (it != ip_map_sessionid.end()){
		memset(&buffer,0,sizeof((int*) buffer));
 		sprintf(buffer,"2");
 		int n = write(newsockfd,buffer,strlen(buffer));
    	if(n<0){
      		error("ERROR writing to socket");
    	}
    	return;
	}
	else
	{
  		char uname[20],passwd[20];
  		int initial;
  		sscanf(buff,"%d %s %s",&initial,uname,passwd);
  	//	printf("Received username passwd from client is:\n" );
  		fputs(uname,stdout);
  		fputs(" ",stdout);
  		fputs(passwd,stdout);
  		printf("\n" );
      
  		FILE *fptr;
      	//cout<<"before open file"<<endl;
  		fptr = fopen(auth_file,"a");
      	//cout<<"after open"<<endl;
   
  		char ch[]="\n";
  		char empty[] = " ";
  		int flag;
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
  		fclose(fptr);
  	}
    
  	if(signupdone ==1 ){
      	cout<<"Client is signed up at server."<<endl;
    	memset(&buffer,0,sizeof((int*) buffer));
    
     	sprintf(buffer,"1");
      
     	n = write(newsockfd,buffer,strlen(buffer));
            
     	if(n<0){
       		error("ERROR writing to socket");
  		}
     
  		return;
  	}

  	else{

     	cout<<"Sorry client could not be signed up at server as same username already exists"<<endl;
    	memset(&buffer,0,sizeof((int*) buffer));
    	sprintf(buffer,"0");
    	n = write(newsockfd,buffer,strlen(buffer));
    	if(n<0){
      		error("ERROR writing to socket");
    	}
    	return;
  	}

  	
 
}

int connect_to_backend(void)
{
	int clientSocket;
	struct addrinfo hints, // Used to provide hints to getaddrinfo()
                    *res,  // Used to return the list of addrinfo's
                    *p;    // Used to iterate over this list
    /* Host and port */
  	const char *host=backend_ip;
  	const char *port=backend_port;
	
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

} //connect_to_backend() ends



int communicate_with_backend_to_receive(int choice,char* file_name,int size,const char* username)
{
  int sockfd = connect_to_backend();
  //cout<<"communicate_with_backend_to_receive() filesize before writing to buffer = "<<size<<endl;
  //cout << "In communicate_with_backend :" << " file name :" << file_name << endl;
  char toSend[BUFF_SIZE];
  bzero(toSend,BUFF_SIZE);
  sprintf(toSend,"%d %d %s %s",choice,size,username,file_name);
  //cout << "toSend:" << toSend << endl;
  int n = send(sockfd,toSend,strlen(toSend),0);
  if (n<=0){
  	cout<<"communicate_with_backend_to_receive() error writing to socket."<<endl;
  }

  return sockfd;      
}


int communicate_with_backend_to_send(int choice,char* file_name,int size,const char* username)

{
	int sockfd = connect_to_backend();
	//cout << "In communicate_with_backend_to_send ()" << " file name = " << file_name << endl;
  	char toSend[BUFF_SIZE];
  	bzero(toSend,BUFF_SIZE);
  	sprintf(toSend,"%d %d %s %s",choice,size,username,file_name);
  	//cout << "toSend:" << toSend << endl;
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

	//cout<<"inside send_file_to_backend\n"<<endl;
	cout<<"Sending file_to_backend\n"<<endl;
  	struct backendArgs *args = (struct backendArgs *)arguments;
	char toSend[BUFF_SIZE];
  	bzero(toSend,BUFF_SIZE);
  	char file_name[50];
  	bzero(file_name,sizeof file_name);

  	memcpy(file_name,args->filename,sizeof(file_name));
  	string username = string(args->userid);
  	int filesize = args->filesize;
  	// update file metadata of user presnet with server.

  
  	int choice = 1;
  	
	//cout << "send_file_to_backend: file name " << file_name << endl;
  
  	int socket=communicate_with_backend_to_send(choice,file_name,filesize,username.c_str());
 
  	string fileLocation = args->filename;
  	//cout << "filelocation: " << fileLocation << endl;
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
  	cout<<"Total bytes sent to backenserver are = "<<sentData<<endl;
  	if(!strcmp(response,"ack")){  // means backend successfullly received file so update metadat of file in server also
  		// metadata file has to be updated once the file is sent to backend
  			FILE *fptr = fopen(username.c_str(),"a");
  			char ch[]="\n";
  			char empty[] = " ";
  			char *filename=NULL;
            filename = strtok(file_name, "_");
    		filename = strtok(NULL, "_");  
  			char* filesizeBuffer = (char *)malloc(sizeof(filesize));
			sprintf(filesizeBuffer,"%d",filesize);
  			fwrite(filename,strlen(filename),1,fptr); // EACH ELEMENT IS OF SIZE 1 BYTE TO BE WRITTEN AND THERE ARE SIZEOF(BUFFER) ELEMENTS
    		fwrite(empty,strlen(empty),1,fptr);
    		fprintf(fptr,"%s",filesizeBuffer);
    		fwrite(ch,strlen(ch),1,fptr);
    		fclose(fptr);
    		free(filesizeBuffer);
    		cout<<"File metadata updated at server local copy also."<<endl;
    
  	} 
  	
  	fclose(sendFile);
  	if( access( fileLocation.c_str(), F_OK ) != -1 ){  // means metafile is present so delete it
		if( remove( fileLocation.c_str() ) != 0 ){
    		perror( "Error deleting file" );
		}
  		else{
    		puts( "Intermediate File successfully deleted from server after sending to backend." );
		}	
		
	}  // access if closed
  	close(socket);
  	pthread_exit(NULL);
  
} // send_file_to_backend() ends

void *receive_file_from_backend(void* arguments){
	cout << "In receive file from backend" << endl;
  	struct backendArgs *args = (struct backendArgs *)arguments;
    char toSend[BUFF_SIZE];
  	bzero(toSend,BUFF_SIZE);
  	char file_name[50];
  	bzero(file_name,sizeof(file_name));
  	memcpy(file_name,args->filename,sizeof(file_name));
  	string username = string(args->userid);
  	int n,choice = 2;
	// this file size is not used 
	int filesize = args->filesize;
  	//cout<<"receive_file_from_backend() filesize passed = "<<filesize<<endl;
    int socket=communicate_with_backend_to_receive(choice,file_name,filesize,username.c_str());
    string fileLocation = args->filename;
	
	long received_file_size_from_backend;
  	char buffer[BUFF_SIZE];
  	bzero(buffer,BUFF_SIZE);
  	n = read(socket,buffer,BUFF_SIZE);
  	sscanf(buffer,"%ld",&received_file_size_from_backend);
  	cout<<"received_file_size_from_backend="<<received_file_size_from_backend<<endl;

  	if(received_file_size_from_backend==0){   // it means file does not exist on backend
  		cout<<"No such file exists on backend"<<endl;
  		close(socket);
  		pthread_exit(NULL);
  		return NULL;
  	}
  	else{   // file exists transfer from backend to server
  		n = write(socket,"filesize_received_ack",strlen("filesize_received_ack"));
  		FILE *receivedFile = NULL;
		receivedFile = fopen(fileLocation.c_str(),"w");
    	if(!receivedFile)
      		fprintf(stderr, "Error fopen ----> %s", strerror(errno));
		int receivedData=0;
        //----buffer chunk to create the file in chunk.
  		char chunk[BUFF_SIZE];
  		bzero(chunk,BUFF_SIZE);
  		int len;

   		while ((len = recv(socket, chunk, BUFF_SIZE, 0)) > 0)
		{
			receivedData+=len;
            if(n= fwrite(chunk, 1,len, receivedFile)<0){
                  error("cant write to file");
            }
            if(receivedData==received_file_size_from_backend)
            {
            	cout << "File received completely from backend Server\n";
                int n = write(socket,"ack",3);
                if (n < 0){
                	error("ERROR writing to socket");
                }
                break;
            }  // if close
 		}  // while close
 
  		cout<<"Total bytes received by server is = "<<receivedData<<endl;
  		fclose(receivedFile);

  		
  	} // else ends
  	close(socket);
  	pthread_exit(NULL);
}   //receive_file_from_backend() ended

void *delete_file_from_backend(void *arguments){
	struct backendArgs *args = (struct backendArgs *)arguments;
    char toDelete[BUFF_SIZE];
  	bzero(toDelete,sizeof((char *)toDelete));
  	char file_name[50];
  	bzero(file_name,sizeof((char *)file_name));
  	memcpy(file_name,args->filename,sizeof(file_name));
  	string username = string(args->userid);
  	int n,choice = 9;
	 // this file size is not used 
	 int filesize = args->filesize;
  	//cout<<"delete_file_from_backend() filesize passed = "<<filesize<<endl;
    int socket=communicate_with_backend_to_receive(choice,file_name,filesize,username.c_str());
    char deleteBuffer[BUFF_SIZE];
    memset(&deleteBuffer,0,sizeof((char *)deleteBuffer));
    
    n = read(socket,deleteBuffer,sizeof((char *)deleteBuffer));
    cout<<"Response from backend after deleting is : "<<deleteBuffer<<endl;
    int response;
    sscanf(deleteBuffer,"%d",&response);
    if(response==1){
    	cout<<"File deleted successfully at backend server."<<endl;
    }
    else{
    	cout<<"File could not be found at backend server."<<endl;
    }
    close(socket);
    pthread_exit(NULL);

} //delete_file_from_backend() ends

void *receive_share_file_from_backend(void* arguments)
{
	cout << "In receive share file from backend" << endl;
  	struct backendArgs *args = (struct backendArgs *)arguments;
  
  	char toSend[BUFF_SIZE];
  	bzero(toSend,BUFF_SIZE);
  	char file_name[50];
  	bzero(file_name,sizeof(file_name));

  	memcpy(file_name,args->filename,sizeof(file_name));
  	string username = string(args->userid);
  	int n;
 
  	int choice = 3;

  	// this file size is not used 

  	int filesize = args->filesize;
  	//cout<<"receive_file_from_backend() filesize passed = "<<filesize<<endl;
  	int socket=communicate_with_backend_to_receive(choice,file_name,filesize,username.c_str());
  	string fileLocation = args->filename;
	long received_file_size_from_backend;
  	char buffer[BUFF_SIZE];
  	bzero(buffer,BUFF_SIZE);
  	n = read(socket,buffer,BUFF_SIZE);
  	sscanf(buffer,"%ld",&received_file_size_from_backend);
  	cout<<"received_file_size_from_backend="<<received_file_size_from_backend<<endl;

  	if(received_file_size_from_backend==0){   // it means file does not exist on backend
  		cout<<"No such file exists on backend"<<endl;
  		close(socket);
  		pthread_exit(NULL); 
		return NULL;
  	}
  	else{   // file exists transfer from backend to server
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
		    	cout << "Requested shared File received completely from backend.\n";
		        int n = write(socket,"ack",3);
		        if (n < 0) error("ERROR writing to socket");
		            break;
		    }
	    }
	 
	  	cout<<"Total bytes received to server is = "<<receivedData<<endl;
	  	fclose(receivedFile);
	  

	} // else ends
	close(socket);
  	pthread_exit(NULL); 

}


void handle_backend(char file_name[50],string userId,long filesize,int choice)
{
	 //cout << "In handle_backend() file name = " << file_name << endl;
	 //cout<<"userId="<<userId<<endl<<"filesize="<<filesize<<endl<<"choice="<<choice<<endl;
  	pthread_t handle_backend;
    //cout<<"hello1"<<endl;
  	struct backendArgs *args;
    //cout<<"hello2"<<endl;
  	args = (backendArgs *)malloc(sizeof(struct backendArgs));
  	//cout<<"hello3"<<endl;
    args->userid = userId;
    //cout<<"args->userid ="<<args->userid<<endl;
    args->filesize = filesize;
    //cout<<"args->filesize ="<<args->filesize<<endl;
  	memcpy(args->filename,file_name,sizeof(args->filename));
  
  	//cout << "args filename: " << args->filename << endl;
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
    { 
		if (pthread_create(&handle_backend, NULL, receive_file_from_backend,args) != 0)
      	{
        	error("Could not create a worker thread");
        	free(args);
      	}
      	pthread_join(handle_backend,NULL);
    }

    else if(choice == 3)
     { cout << "Inside else if" << endl; 

      if (pthread_create(&handle_backend, NULL, receive_share_file_from_backend,args) != 0)
      {
        error("Could not create a worker thread");
        free(args);
      }
      pthread_join(handle_backend,NULL);
    }

    else if(choice == 9){
    	if (pthread_create(&handle_backend, NULL, delete_file_from_backend,args) != 0)
      	{
        	error("Could not create a worker thread");
        	free(args);
      	}
      	pthread_join(handle_backend,NULL);

    }

     
    free(args);
     

}  //handle_backend() closed


void send_file(int socket,char file_name[50],string userId)
{

    //cout << "In send_file\n";
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
        	printf("--\b");
            len=send(socket,chunk,len,0);
                        
            sentData+=len;
           /* if(sentData == filesize){
                    break;
                }  */

        }
        cout<<endl;
    cout<<"Server sent "<<sentData<<" bytes to client successfully !!"<<endl;
    fclose(sendFile);
   
  
} 

int receive_file(int socket,char file_name[50],string userId,long filesize)
{
	string fileLocation = string(file_name);
    //cout << "file name: " << file_name << "userId: " << userId << "fileLocation:" << fileLocation << endl;
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
                
        if(receivedData==filesize){
          	cout<<"Upload Complete at Server"<<endl;
      		// send ack to client 
    		int n = write(socket,"ack",3);
            if (n < 0) 
               	error("ERROR writing to socket");
                break;
        }
    } // while closed

      	fclose(receivedFile);
      	return 1;
}  


int if_multiples_uploads(char* filename,char clientip[50])
{
	string username = ip_map_uname[clientip];
    char filename_inside_file[50];
    char line[50];
    if( access(username.c_str(), F_OK ) != -1 )
    {
    	FILE *fptr = fopen(username.c_str(),"r");
        int is_file_available = 0;
        long filesize;
        while(fgets(line,sizeof(line),fptr)!= NULL)
        {
            sscanf(line,"%s %ld",filename_inside_file,&filesize);
            if(!strcmp(filename_inside_file,filename))
            {  // if filename match found then make a call to backend else return to client saying no such file exists.
                return 0;
            }
        } //while
    }
    return 1;
}  //if_multiples_uploads() ends

void receive_from_client(int newsockfd, char buffer[BUFF_SIZE],char clientip[50])
{
	char filename[50];
	int n;
	int initial;
	int sid;
	long int filesize;
    sscanf(buffer,"%d %d %s %ld",&initial,&sid,filename,&filesize);
    //cout << "In receive_from_client: filename " << filename << endl;	
  	int sessionactiveflag=checksessionactive(clientip,sid);
    int iffilemultipleupload=if_multiples_uploads(filename,clientip);
    
  	bzero(buffer,BUFF_SIZE);
    if(sessionactiveflag==1 && iffilemultipleupload == 1)
    {
  		printf("session match found at server\n");
  		sprintf(buffer,"%d",sessionactiveflag);
  		n = write(newsockfd,buffer,strlen(buffer));
		if(n<0)
        	error("Error writing to socket");
  	
    	string username = ip_map_uname[clientip];

    	string fileName = string(username) + "_" + string(filename);
      	//cout << "username: " << username << " filename: " << fileName << endl;
    	int status = receive_file(newsockfd,(char *)fileName.c_str(),username,filesize);
    
   	 	if(status){
   	 		// update filename in username metadata file.  
			handle_backend((char *)fileName.c_str(),username,filesize,1);
    	} //

    	else
      		printf("could not receive file from client\n");
	 }

  	// else notify client to login first
  	else if(!sessionactiveflag)
  	{
  		printf("session match not found at server\n");
  		sprintf(buffer,"%d",sessionactiveflag);
  		n = write(newsockfd,buffer,strlen(buffer));
  		if(n<0)
  			error("Error writing to socket");
  		
	   	}  // else if closed
  	else if(!iffilemultipleupload)
  	{
    	printf("File has been uploaded already\n");
      	sprintf(buffer,"%d",2);
      	n = write(newsockfd,buffer,strlen(buffer));
      	if(n<0)
        	error("Error writing to socket");
  	}


} // receive_from_client() closed


void send_to_client(int newsockfd, char buffer[BUFF_SIZE],char clientip[50],int priv_share)
{
	//cout<<"inside send to client"<<endl;
  	char filename[50];
  	bzero(filename,sizeof(filename));
  	int n;
  	int initial;  // 4 sid filename
  	int sid;
  	long int filesize=0;
  	sscanf(buffer,"%d %d %s",&initial,&sid,filename);
  	//cout<<"received filename at server to send to client is:"<<filename<<endl;
  	int sessionactiveflag=checksessionactive(clientip,sid);
  	bzero(buffer,BUFF_SIZE);
  	string username = ip_map_uname[clientip];
  	cout << "Sending file to client:"<< username.c_str() << endl;
  	string fileName = string(username) + "_" + string(filename);
    
  	if(sessionactiveflag==1)
    {   // means session is active
    	printf("session match found at server\n");
    	char fname[50];
		  bzero(fname, sizeof fname);
		  if(priv_share)
    		  memcpy(fname,ip_map_uname[clientip].c_str(),sizeof(fname));
    	else
    		  memcpy(fname,shared_file,sizeof(fname));
    	int is_file_available=0;
    	cout<<"fname is:"<<fname<<endl;
    	if( access( fname, F_OK ) != -1 )
      {  // means metadata file or share.txt is available openit and check for requested file name
    		cout<<"inside access"<<endl;
    		char filename_inside_file[50];
    		long filesize;
    		char userid[50];
    		char line[50];
    		FILE *fptr = fopen(fname,"r");
      		if(priv_share)
      		{
      			while(fgets(line,sizeof(line),fptr)!= NULL)
      			{
  	    			sscanf(line,"%s %ld",filename_inside_file,&filesize);
  	    			if(!strcmp(filename_inside_file,filename))
      				{  // if filename match found then make a call to backend else return to client saying no such file exists.
        					cout<<"file is found in metadata at server."<<endl;
        					is_file_available = 1;
        					break;
        				}

    				} //while ends
      		}
    		else
    		{

    			while(fgets(line,sizeof(line),fptr)!= NULL)
    			{
	    			sscanf(line,"%s %s",filename_inside_file,userid);
	    			if(!strcmp(filename_inside_file,filename))
    				{  // if filename match found then make a call to backend else return to client saying no such file exists.
      				cout<<"file is found in share.txt at server."<<endl;
      				is_file_available = 1;
      				break;
      				}
    			} //while 
    		}
  			fclose(fptr);
  		} // if close
  	
  		

  
  
  	else{ 
			cout<<"Metadata file is not found in server.so client has to upload files first."<<endl;
   			char buffer[BUFF_SIZE];
  			bzero(buffer,BUFF_SIZE);
  			int filesize = 0;
  			int response = 2;  // 2 means metadata file is not present at backend so client has to upload something first.  
  			sprintf(buffer,"%d %d",response,filesize);
      		int n = write(newsockfd,buffer,strlen(buffer));
      		if(n<0){
        		error("send_to_client() Error writing to socket");
      		}
      		return;
  	}

  	if (is_file_available == 1)
    {
  			if(priv_share)
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
            if(n<0)
                error("Error writing to socket");
              

            bzero(buffer,BUFF_SIZE);
            n = read(newsockfd,buffer,BUFF_SIZE);
            cout<<"Response from client after sending file size is:"<<buffer<<endl;
            send_file(newsockfd,(char *)fileName.c_str(),username);
            cout << "sent complete file to client\n";
            bzero(buffer,BUFF_SIZE);
            n = read(newsockfd,buffer,3);
            cout << "message from client : " << buffer << endl;
            if( access( fileName.c_str(), F_OK ) != -1 )
              {  // means metafile is present so delete it
                if( remove( fileName.c_str()) != 0 )
                {
                  perror( "Error deleting file" );
                }
                else{
              puts( "Intermediate File successfully deleted from server after sending to client." );
                    } 
      
             }  // access if closed
        
        }
  			else
        {
        		cout << "before handle backend in else\n";
  				handle_backend((char *)filename,username,filesize,3);


          FILE *fptr1 = NULL;

      		fptr1 = fopen(filename,"r");
      		fseek(fptr1,0, SEEK_END);
      		long file_len =(unsigned long)ftell(fptr1);
      		printf("length of file is%ld\n",file_len);
      		fseek(fptr1,0,SEEK_SET);
      		fclose(fptr1);

          	sprintf(buffer,"%d %ld",sessionactiveflag,file_len);
          	n = write(newsockfd,buffer,strlen(buffer));
          	if(n<0)
              	error("Error writing to socket");
              

          	bzero(buffer,BUFF_SIZE);
          	n = read(newsockfd,buffer,BUFF_SIZE);
          	cout<<"Response from client after sending file size is:"<<buffer<<endl;
        		send_file(newsockfd,(char *)filename,username);
        		cout << "sent complete file to client\n";
        		bzero(buffer,BUFF_SIZE);
        		n = read(newsockfd,buffer,3);
        		cout << "message from client : " << buffer << endl;

        		cout<<"Removing sent file from server."<<endl;
        		if( access( filename, F_OK ) != -1 )
              {  // means metafile is present so delete it
  				      if( remove( filename) != 0 )
                {
      				    perror( "Error deleting file" );
  				      }
    				    else{
      				puts( "Intermediate File successfully deleted from server after sending to client." );
  				          }	
  		
  			     }  // access if closed
          } // else closed

		 }  //is_file_available if closed


      	else if(is_file_available==0)
      	{

      		int reponse = 2; // it indicates that the requested file is not available at backend server also.
      		sprintf(buffer,"%d",reponse);
      		n = write(newsockfd,buffer,strlen(buffer));
      		if(n<0){
        		error("send_to_client() Error writing to socket");
      		}
      	} 
      } // session active flag if closed 
    
    
    else{
    	// session is not active
    	cout<<"session match failed"<<endl;
      	long filesize = 0;
      	sprintf(buffer,"%d %ld",sessionactiveflag,filesize);
      	n = write(newsockfd,buffer,strlen(buffer));
      	if(n<0){
        	error("Error writing to socket");
      	}
    } // else close


} //send_to_client() closed 


// clear_session 
void clear_session(int newsockfd, char buff[BUFF_SIZE], char clientip[50]){
	int session_cleared=0;
	int client_sessionid,choice;
	sscanf(buff,"%d %d",&choice,&client_sessionid);
	
	if( access( ip_map_uname[clientip].c_str(), F_OK ) != -1 ){  // means metafile is present so delete it
		if( remove( ip_map_uname[clientip].c_str() ) != 0 ){
    		perror( "Error deleting file" );
		}
  		else{
    		puts( "User Metadata File successfully deleted" );
		}	
		
	}  // access if closed
	if (ip_map_sessionid.count(clientip)>0){
		if(ip_map_sessionid[clientip]==client_sessionid){
			session_cleared=1;
	 		ip_map_uname.erase (clientip);
	 		ip_map_sessionid.erase (clientip);
	 		cout<<"Map cleared for user."<<endl;
		}
	}
	char buffer[BUFF_SIZE];
	bzero(buffer,BUFF_SIZE);
	sprintf(buffer,"%d",session_cleared);
	//cout<<"logout():value before witing buffer is:"<<buffer;
	int n = write(newsockfd,buffer,strlen(buffer));
    if(n<0){
  		error("Error writing to socket");
    }
    close(newsockfd);
    pthread_exit(NULL);
}


//this function gets the list of files available with backend for a particular client and stores them temporarily here.
void share_filename_with_backend(int newsockfd,char buffer[BUFF_SIZE],char clientip[50])
{
	int choice;
	int sessionid;
	char filename[50];
	char username[50];
	memcpy(username,ip_map_uname[clientip].c_str(),sizeof(username));
	sscanf(buffer,"%d %d %s",&choice,&sessionid,filename);

  	int flag = checksessionactive(clientip,sessionid);
  	string fileLocation = shared_file;
  	
	if(flag)
	{
			char line[256];
  			char filename_inside_file[50];
  			long filesize;
  			char username_inside_file[50];
  			//means metadata file of user is present at server
  			if( access(username, F_OK ) != -1 )
    		{

        		FILE *fptr = fopen(username,"r");
        		while(fgets(line,sizeof(line),fptr)!= NULL)
        		{
                	sscanf(line,"%s %ld",filename_inside_file,&filesize);
                	if(!strcmp(filename_inside_file,filename))
                  	{
                  		flag = 2;  // flag 2 means file is already uploaded
                  		break;

                  	}
                  	else{
                  		flag =50;   // flag 50 means requested file is not yet uploaded
                  	}
        		}
        		fclose(fptr); 
        		
        	
    		}
    		else{
    			flag = 10;  // metafile does not exist which means no file is uploaded
    		}
    		pthread_mutex_lock (&sharedfilelock);
			if( (flag==2) && access(shared_file, F_OK ) != -1 )
    		{
    			FILE *fptr = fopen(shared_file,"r");
        		while(fgets(line,sizeof(line),fptr)!= NULL)
        		{
        			sscanf(line,"%s %s",filename_inside_file,username_inside_file);
            		if(!strcmp(filename_inside_file,filename) && !strcmp(username_inside_file,username))
            		{
            			flag = 3;   // 3 means file is already shared
            			break;
            		}
        		}
        		fclose(fptr); 
    		}
    		pthread_mutex_unlock (&sharedfilelock);
			
  	}
	else{
		printf("session match not found at server\n");
		flag = 0;
	}
  			
  	if((flag!=3)&& (flag==2)&&(flag!=10)&&(flag!=0)  ){
  		pthread_mutex_lock (&sharedfilelock);
		FILE *shareFile = NULL;
    	shareFile = fopen(fileLocation.c_str(),"a");
    	char ch[] = "\n";
    	char empty[] = " ";
    	fwrite(filename,strlen(filename),1,shareFile); 
    	fwrite(empty,strlen(empty),1,shareFile);
    	fwrite(username,strlen(username),1,shareFile);
    	fwrite(ch,strlen(ch),1,shareFile);
    	fclose(shareFile);
    	pthread_mutex_unlock (&sharedfilelock);
    	communicate_with_backend_to_send(choice,filename,0,username);
    	flag = 5; //means file is successfully shared
  	}

  	

    char buff[BUFF_SIZE];
    bzero(buff,sizeof(buff));
    sprintf(buff,"%d",flag);
     
    int n = write(newsockfd,buff,BUFF_SIZE);
    if(n<=0){
      error("share file name with backend error writing to socket.");
  	}

}

void show_filesystem_to_client(int newsockfd,char buffer[BUFF_SIZE],char clientip[50]){
	int choice,sessionid;
	cout<<"inside get_filesystem_from_backend()"<<endl;
	sscanf(buffer,"%d %d",&choice,&sessionid);
	cout<<"sessionid = "<<sessionid<<endl;
	if(checksessionactive(clientip,sessionid)){  // means session is active
		cout<<"Session match found at server."<<endl;
		char fname[50];
		bzero(fname, sizeof fname);
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
      		

		} 		
		else 
		{
			cout<<"No metadata file exists for this client."<<endl;
			char buffer[BUFF_SIZE];
      		bzero(buffer,BUFF_SIZE);
      		int choice = 2;
      		long file_len =0;
      		sprintf(buffer,"%d %ld",choice,file_len);
      		int n = write(newsockfd,buffer,strlen(buffer));
        	if(n<0){
            	error("Error writing to socket");
            }

            return;
    } // else closed
  
    

	} // checksessionactive if closed  

	else
	{
		// session active check failed
		cout<<"Session check failed for this request."<<endl;
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


void show_sharedfile_to_client(int newsockfd,char buffer[BUFF_SIZE],char clientip[50]){
  int choice,sessionid;
  cout<<"inside show_sharedfile_to_client()"<<endl;
  sscanf(buffer,"%d %d",&choice,&sessionid);
  cout<<"sessionid = "<<sessionid<<endl;
  if(checksessionactive(clientip,sessionid)){  // means session is active
    cout<<"Session match found at server."<<endl;
    char fname[50];
    bzero(fname, sizeof fname);
    string dirname = shared_file;
    int choice =1;
    memcpy(fname,dirname.c_str(),sizeof(fname));
    cout<<"fname="<<fname<<endl;
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
          

    }     
    else 
    {
      cout<<"No shared file exists for this client."<<endl;
      char buffer[BUFF_SIZE];
          bzero(buffer,BUFF_SIZE);
          int choice = 2;
          long file_len =0;
          sprintf(buffer,"%d %ld",choice,file_len);
          int n = write(newsockfd,buffer,strlen(buffer));
          if(n<0){
              error("Error writing to socket");
            }

            return;
    } // else closed
  
    

  } // checksessionactive if closed  

  else
  {
    // session active check failed
    cout<<"Session check failed for this request."<<endl;
    int response = 0;
    char *responsebuffer = NULL;
    char buffer[BUFF_SIZE];
    bzero(buffer,BUFF_SIZE);
    sprintf(buffer,"%d %s",response,responsebuffer);
    int n =write(newsockfd,buffer,strlen(buffer));
    if(n<0){
      cout<<"show_sharedfile_to_client():Error writing to socket"<<endl;
    }

  }  // else closes
  return;

  
} // get_filesystem() closed


int getFileSize(const std::string &fileName)
{
    ifstream file(fileName.c_str(), ifstream::in | ifstream::binary);

    if(!file.is_open())
    {
        return -1;
    }

    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.close();

    return fileSize;
}
int deleteFilename(char* filename,const char* filelocation)
{
	int flag = 2;
 	FILE *fptr = fopen(filelocation,"r");
  //cout<<"inside deleteFilename(): fileLocation="<<filelocation<<endl;
  //cout<<"inside deleteFilename(): fileto delete="<<filename<<endl;
 	ofstream temp;
 	temp.open("temp.txt");
   	char filename_inside_file[50];
    char userid[50];
    char line[50];

    while(fgets(line,sizeof(line),fptr)!= NULL)
            {
            sscanf(line,"%s %s",filename_inside_file,userid);

            
            if(strcmp(filename_inside_file,filename))
                temp << line;
            else{
            	flag = 1;
            }
        

            } //while
	  temp.close();
    fclose(fptr);
    remove(filelocation);
    rename("temp.txt",filelocation);
   
    //cout<<"end of deleteFilename()"<<endl;
    return flag;
}

void deletefile(int newsockfd, char delete_buffer[BUFF_SIZE],char clientip[50]){
	// this function will remove the specified filename in buffer from username file and share.txt file and also from backend share.txt and the original file on the backend.
	cout<<"in deletefile()"<<endl;
	int choice,n;
	char file_to_delete[50];
	int sessionid;
  bzero(file_to_delete,sizeof file_to_delete);
	sscanf(delete_buffer,"%d %d %s",&choice,&sessionid,file_to_delete);
  //cout<<"after reading from sscanf"<<endl;
  //cout<<"choice = "<<choice<<"sessionid="<<sessionid<<endl<<"file_to_delete="<<file_to_delete<<endl;
	memset(&delete_buffer,0,sizeof((int*)delete_buffer));
	int feedback_to_client = 0;   // 0 means sesssion is not active , 1 means session is active and file is deleted successfully, 2 requested file is not uploaded on server, 
  string metafile = ip_map_uname[clientip].c_str();
  string shared_file = shared_file;
	if(checksessionactive(clientip,sessionid)){
		
		if( access( metafile.c_str(), F_OK ) != -1 ) {   
			// means meta file is present. open it find the filename and delete.
			//FILE *fptr = fopen(metafile,"w");
			cout<<"metafile ="<<metafile<<" is present"<<endl;
			feedback_to_client = deleteFilename(file_to_delete,metafile.c_str());
			int n=getFileSize(metafile.c_str());
			if(n==0){
        cout<<"metafile size after deletion is "<<n<<endl;
			
				if( remove( metafile.c_str() ) != 0 ){
    				perror( "Error deleting file" );
				}
  				else{
    				puts( "Metadata File successfully deleted from server as no more entry left to compare while deleting." );
				}	
			}

		
			// check in shared file also
			
			if( access( shared_file.c_str(), F_OK ) != -1 ) {  
		 	// means shared file is present. open it find the filename and delete.
        cout<<"sharedfile ="<<shared_file<<" is presnt"<<endl;
			   //cout<<"inside access shared_file"<<endl;
				int x = deleteFilename(file_to_delete,shared_file.c_str());
				int y=getFileSize(shared_file.c_str());
				if(y==0){
          cout<<"shared_file size is ="<<y<<endl;
					if( remove( shared_file.c_str() ) != 0 ){
    					perror( "Error deleting file" );
					}
  					else{
    					puts( "Shared File successfully deleted from server as no more entry left to compare while deleting." );
					}	

				}
			}	

      //cout<<"before feedback_to_client == 1 condition"<<endl;
			// create backend thread
			if(feedback_to_client==1){
				//cout<<"before handle_backend() calling"<<endl;
				//cout<<"file_to_delete = "<<file_to_delete<<endl;
				//cout<<"metafile = "<<metafile<<endl;
				//cout<<"choice = "<<choice<<endl;
        cout<<"before calling handle_backend"<<endl;
				handle_backend(file_to_delete,metafile,0,choice);
				cout<<"after handle_backend() completed back in deletefile."<<endl;
			
				char response_buffer[BUFF_SIZE];
				memset(&response_buffer,0,sizeof((char *)response_buffer));
				sprintf(response_buffer,"%d",feedback_to_client);
				n = write(newsockfd,response_buffer,strlen(response_buffer));
				if(n<0){
					error("deletefile() Error writing to socket.");
				}
				//cout<<"before if closed."<<endl;
				return;
			}
			// means file does not exists on server
			if(feedback_to_client==2){
				char response_buffer[BUFF_SIZE];
				memset(&response_buffer,0,sizeof((char *)response_buffer));
				sprintf(response_buffer,"%d",feedback_to_client);
				n = write(newsockfd,response_buffer,strlen(response_buffer));
				if(n<0){
					error("deletefile() Error writing to socket.");
				}
				cout<<"Requested file is not uploaded on server."<<endl;
				return;

			}


		} //  access if closed

		else{

			feedback_to_client = 2; 
			char response_buffer[BUFF_SIZE];
			memset(&response_buffer,0,sizeof((char *)response_buffer));
			sprintf(response_buffer,"%d",feedback_to_client);
			n = write(newsockfd,response_buffer,strlen(response_buffer));
			if(n<0){
				error("deletefile() Error writing to socket.");
			}
			cout<<"Requested file is not uploaded on server."<<endl;
			return;

		}
		
		
			 
		
		

	}  // if closed

	
	else{  // session not active
		cout<<"Sorry you are not logged in"<<endl;
		//feedback_to_client = 2; 
		char response_buffer[BUFF_SIZE];
		memset(&response_buffer,0,sizeof((char *)response_buffer));
		sprintf(response_buffer,"%d",feedback_to_client);
		n = write(newsockfd,response_buffer,strlen(response_buffer));
		if(n<0){
			error("deletefile() Error writing to socket.");
		}
			
			return;
	}	 // else closed
	//cout<<"before closing deletefile()"<<endl;

}  //deletefile() closed


void *service_single_client(void *args){
	struct clientArgs *wa;
	int newsockfd;  

    /* Unpack the arguments */
    wa = (struct clientArgs*) args;
    newsockfd = wa->socket;

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



	while(0){
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
      			//pt(1);
      			signupuser(newsockfd,buffer,ipstr);
      			break;
    		}
			case 3:
			{
				receive_from_client(newsockfd,buffer,ipstr);
				break;
			}
			case 4:
			{
				show_filesystem_to_client(newsockfd,buffer,ipstr);
				break;
			}
			case 5:
			{
				send_to_client(newsockfd,buffer,ipstr,1);
				break;
			}
			case 6:
			{
				//communicate_with_backend_to_send(choice,file_name,filesize,username.c_str());
				share_filename_with_backend(newsockfd,buffer,ipstr);
				break;
			}
        case 7:
      {
        send_to_client(newsockfd,buffer,ipstr,0);
        break;
      }
        case 8:
      {
        show_sharedfile_to_client(newsockfd,buffer,ipstr);
        break;
      }
			case 9:
			{
				deletefile(newsockfd,buffer,ipstr);
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
  backend_ip = argv[2];
  backend_port = argv[3];
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
    //cout<<"wa->socket  "<<wa->socket<<endl;

  
  	if (pthread_create(&client_thread, NULL, service_single_client, wa) != 0) 
        {
            perror("Could not create a worker thread");
            free(wa);
            close(newsockfd);
  			//close(sockfd);
            pthread_exit(NULL);
        }
       pthread_detach(client_thread);
	} // while ends

	close(sockfd);
   
	return 0;
}
