#include<stdio.h>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<time.h> 
#include<string.h>
#include<iostream>
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

//code to upload file to server for backup

void upload(int sockfd){
  char  filename[50];
  cout<<"enter the filename to upload:";
  cin>>filename;
  char buffer[BUFFER_SIZE];
  bzero(buffer,BUFFER_SIZE);
  int uploadfilenamecode = 31;  // 31 is the code for uploading file name to server and authenticating with session id before uplaoding.
  int n;
  sprintf(buffer,"%d %d %s",uploadfilenamecode,sessionid,filename);
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

    // function to upload file 
  }
  else{
    cout<<"Session ID match failed. Please login first.";
    return;
  }


}

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
          else{
            printf("Client cannot be authenticated at server end.\n" );
          
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
            upload(sockfd);

          }


    }


  }while(choice!=0);
  //printf("%s\n",buffer);
  return 0;
}
