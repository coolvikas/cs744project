#include<stdio.h>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<time.h> 
#include <fcntl.h> /* O_WRONLY, O_CREAT */
#include <unistd.h> /* close, write, read */
#include<string.h>
#include<iostream>
#include<cmath>
#include<stdlib.h> //for exit system call
#include<unistd.h> //for read and write functions
#define BUFFER_SIZE 256
int sessionid;

using namespace std;
void error(const char *msg){
  perror(msg);
  exit(0);
}

char* loginuser(){
  char *buffer=(char *)(malloc(sizeof(char)*BUFFER_SIZE));
  char uname[20], password[20];
  printf("Enter username: " );
  scanf("%s",uname );
  printf("Enter password: " );
  scanf("%s",password );
  bzero(buffer,256);
  printf("uname and password entered are : %s %s\n",uname,password );
  int choice=1;
  sprintf(buffer,"%d %s %s",choice,uname,password);
  return buffer;
}

char* signupuser(){
  char *buffer=(char *)(malloc(sizeof(char)*BUFFER_SIZE));
  char uname[20], password[20];
  printf("Enter username: " );
  scanf("%s",uname );
  printf("Enter password: " );
  scanf("%s",password );
  bzero(buffer,256);
  printf("uname and password entered are : %s %s\n",uname,password );
  int choice=2;
  sprintf(buffer,"%d %s %s",choice,uname,password);
  return buffer;

}

int send_file(int sock, char *file_name)
{
 int sent_count; /* how many sending chunks, for debugging */
 int read_bytes, /* bytes read from local file */
 sent_bytes, /* bytes sent to connected socket */
 sent_file_size;
 char send_buf[BUFFER_SIZE]; /* max chunk size for sending file */
 const char * errmsg_notfound = "File not found\n";

 int f; /* file handle for reading local file*/
 sent_count = 0;
 sent_file_size = 0;
 /* attempt to open requested file for reading */
 if( (f = open(file_name, O_RDONLY)) < 0) /* can't open requested file */
 {

 error(file_name);
 //if( (sent_bytes = send(sock, errmsg_notfound ,strlen(errmsg_notfound), 0)) < 0 ){
 // perror("send error");
 // return -1;
 // }
 }

 else /* open file successful */
 {
  /*

  long partitions = ceil(file_len/float(BUFFER_SIZE));
  cout<<"partitions calculated:"<<partitions<<endl ;
  char buffer1[BUFFER_SIZE];
  //int eof = EOF;
  //cout<<"eof= "<<eof<<endl;
  bzero(buffer1,BUFFER_SIZE);
  sprintf(buffer1,"%ld",partitions);
  cout<<"writing partitions to sever socket"<<endl;
  int n = write(sock,buffer1,strlen(buffer1));
  if(n<0){
     error("ERROR writing to socket");
  } 
  char replybuffer[BUFFER_SIZE];
  bzero(replybuffer,BUFFER_SIZE);
  n = read(sock,replybuffer,sizeof(replybuffer));
  cout<<"server reply after sending partitions is:"<<replybuffer<<endl;   */
  
  printf("Sending file: %s\n", file_name);
  //sleep(2);
 while( (read_bytes = read(f, send_buf, BUFFER_SIZE)) > 0 )
 {
 if( (sent_bytes = send(sock, send_buf, read_bytes,0))< read_bytes )
 {
 error("send error");
 return -1;
 }
 sent_count++;
 sent_file_size += sent_bytes;
 }
 close(f);
 } /* end else */
 char response[20];
 bzero(response,20);
 int n = read(sock,response,20);
 cout<<"response from server after sending all chunks at client end is "<<response<<endl;
 printf("Done with this client. Sent %d bytes in %d send(s)\n\n",sent_file_size, sent_count);
//return sent_count;
}

//code to upload file to server for backup

int upload(int sockfd){
  char  filename[50];
  cout<<"enter the filename to upload:";
  cin>>filename;
  char buffer[BUFFER_SIZE];
  bzero(buffer,BUFFER_SIZE);
  int uploadfilenamecode = 3;  // 3 is the code for uploading file name to server and authenticating with session id before uplaoding.
  int n;
  FILE *fptr;
  fptr=fopen(filename,"r");
  cout<<"fptr"<<fptr;
  if(fptr==NULL){
    error("NO such filename exists");
    return 0;
  }
  fclose(fptr);
   FILE *fptr1;
  fptr1 = fopen(filename,"r");
  fseek(fptr,0, SEEK_END);
    unsigned long file_len =(unsigned long)ftell(fptr1);
    printf("length of file is%ld\n",file_len);
  fseek(fptr1,0,SEEK_SET);
  fclose(fptr1);


  sprintf(buffer,"%d %d %s %ld",uploadfilenamecode,sessionid,filename,file_len);
  printf("upload():Value before writing  buffer is:%s\n",buffer );
  n = write(sockfd,buffer,strlen(buffer));
  if(n<0){
      error("ERROR writing to socket");
  }
  bzero(buffer,BUFFER_SIZE);
  n = read(sockfd,buffer,BUFFER_SIZE);
  if(n<0){
      error("ERROR reading from socket");
  }
  cout<<"upload():reponse in buffer from server of 31 is:"<<buffer<<endl;
  int sessionidresponse;
  sscanf(buffer,"%d",&sessionidresponse);
  if(sessionidresponse==1){
    cout<<"Session ID matched at server. Starting file upload to server."<<endl;
    send_file(sockfd,filename);

    // function to upload file 
  }
  else{
    cout<<"Session ID match failed. Please login first.";
    return 0;
  }


}

//to logout the client from server and removes its session also.
void logout(int sockfd){
  char buffer1[BUFFER_SIZE];
  bzero(buffer1,BUFFER_SIZE);
  int choice = 5;
  sprintf(buffer1,"%d %d",choice,sessionid);
  int n = write(sockfd,buffer1,strlen(buffer1));
  if(n<0){
      error("ERROR writing to socket");
  }
  bzero(buffer1,BUFFER_SIZE);
  n = read(sockfd,buffer1,BUFFER_SIZE);
  if(n<0){
      error("ERROR reading from socket");
  }
  int logout_response;
  sscanf(buffer1,"%d %d",&logout_response,&sessionid);
  if(logout_response==1){
    cout<<"User successfully logged out from system."<<endl;
  }
  else{
    cout<<"U are not logged in."<<endl;
  }
}


// This section will download file from remote server. First it will send message request "4 sessionid filename". 
// then middle server will check if client is logged in or not via session id and give response.
int download(int sockfd){
  //get filename from user
  cout<<"inside download()"<<endl;
  char filename[50];
  cout<<"Enter filename : ";
  cin>>filename;
  //form a message request in downlaodbuffer
  char download_buffer[BUFFER_SIZE];
  bzero(download_buffer,BUFFER_SIZE);
  int choice = 4;   // here 4 is code for downlaod
  //enter choice sessionid filename in buffer and send to socket 
  sprintf(download_buffer,"%d %d %s",choice,sessionid,filename);
  int n = write(sockfd,download_buffer,strlen(download_buffer));
  if(n<0){
    cout<<"Error writing to download_buffer socket to server"<<endl;
  }
  bzero(download_buffer,BUFFER_SIZE);
  // read response from server in download_buffer
  n = read(sockfd,download_buffer,sizeof (download_buffer));
  if(n<0){
    cout<<"Error reading server response in download_buffer"<<endl;
  }
  int download_response;
  long download_filesize;
  sscanf(download_buffer,"%d %ld",&download_response,&download_filesize);
  if(download_response==0){
    cout<<"Sorry you are not logged in. Please login first."<<endl;
    return -1 ;
  }
  else{
    cout<<"Requested file is found at server !!"<<endl;
    // file match is found on server and filesize is received so ack server that client is ready to download file.
    int n = write(sockfd,"filesize_received_ack",21);
    if(n<0){
    cout<<"Error writing filesize_received_ack to server socket."<<endl;
    }
    //open a file for writing
    int f; //file descriptor
    ssize_t rcvd_bytes, rcvd_file_size;
    int recv_count; 
    char recv_str[BUFFER_SIZE]; 
    recv_count = 0; /* number of recv() calls required to receive the file */
    rcvd_file_size = 0; /* size of received file */
 
    if ( (f = open(filename, O_WRONLY|O_CREAT, 0644)) < 0 )
    {
      error("error creating file");
 
    }
    cout<<"successfully opened file for witing."<<endl;
    cout<<"Receiving data from server..";
    //cout<<"test after opening file in write mode"<<endl;
    while ((rcvd_bytes = recv(sockfd, recv_str, BUFFER_SIZE,0)) > 0){
      cout<<".";
      recv_count++;
      rcvd_file_size += rcvd_bytes;

      if (write(f, recv_str, rcvd_bytes) < 0 ){
         error("error writing to file");
      }  
      if(rcvd_file_size==download_filesize)
      {
          int n = write(sockfd,"ack",3);
          if (n < 0) error("ERROR writing to socket");
          break;
      }
    }  //while close
    close(f); /* close file*/
    cout<<"Client Downloaded:"<<rcvd_file_size<<" bytes in "<<recv_count<<" recv(s)\n"<<endl;
  }  // close else


} // close download



int main(int argc, char *argv[])
{
  int sockfd, portno ,n;

  struct sockaddr_in server_addr;
  struct hostent *server;

  if(argc<3){
    fprintf(stderr,"usage %s hostname port\n", argv[0] );
    exit(0);
  }
  portno = atoi(argv[2]);
  sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd<0){
    error("Error opening socket");
  }
  else{
    printf("Socket opened successfully\n");
  }

  server = gethostbyname(argv[1]);
  if(server == NULL){
    fprintf(stderr, "ERROR no such host\n" );
    exit(0);
  }
  bzero((char *)&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);
  server_addr.sin_port = htons(portno);
  if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
    error("ERROR connecting");
  }
  int choice;
  do {
    /* code */
    printf("Please select an option: (0 to exit)\n" );
    printf("1.Login\n" );
    printf("2.Signup\n" );
    printf("3.Upload Files\n");
    printf("4.Downlaod Files\n");
    printf("5.Logout\n");

    scanf("%d",&choice );
    
    switch(choice)
    {
        
        case 1: //loginuser
          ;
          char *buffer1;
          buffer1 = loginuser();
          printf("Value before writing  buffer is:%s\n",buffer1 );
          n = write(sockfd,buffer1,strlen(buffer1));
          if(n<0){
            error("ERROR writing to socket");
          }
          char buffer[BUFFER_SIZE];
          bzero(buffer,256);
          n = read(sockfd,buffer,255);
          if(n<0){
            error("ERROR reading from socket");
          }
          cout<<"buffer received from server after calling login is"<<buffer<<endl;
          int status;
          sscanf(buffer,"%d %d",&status,&sessionid);
          if(status == 1){
            printf("Client is successfully authenticated at server end.\n");
            cout<<"Session ID is:"<<sessionid;
          }
          else if(status ==0){
            printf("Client cannot be authenticated at server end.\n" );
          
          }
          if(status == 2)
          {
            cout<<"client is already logged in with session id"<<sessionid<<" PLease do other operations.";
          }
          break;


        case 2: //lets signup first
        {
            char *buffer1;
            buffer1 = signupuser();;
            printf("signupuser():Value before writing buffer is:%s\n",buffer1 );
            n = write(sockfd,buffer1,strlen(buffer1));
            if(n<0){
              error("ERROR writing to socket");
            }
            bzero(buffer,256);
            n = read(sockfd,buffer,255);
            if(n<0){
              error("ERROR reading from socket");
            }
            if(!strcmp(buffer,"1")){
              printf("Client is successfully signed up at server end. U may login now.\n");
            }
            else{
              printf("Please try again signup with different username and password.\n" );
            }
            break;
        }
        case 3:
        {
            int x = upload(sockfd);
            break;

        }
        case 4:
        {
          
          int flag = download(sockfd);
          break;
        }
        case 5:
        {
            logout(sockfd);
            break;

        }


    }


  }while(choice!=0);
  //printf("%s\n",buffer);
  return 0;
}
