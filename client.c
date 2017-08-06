#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h> //for exit system call
#include<unistd.h> //for read and write functions
#define BUFFER_SIZE 256


void error(char *msg){
  perror(msg);
  exit(0);
}

char* loginuser(){
  char *buffer=malloc(sizeof(char)*BUFFER_SIZE);
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
  char *buffer=malloc(sizeof(char)*BUFFER_SIZE);
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
    printf("3.View Files\n");
    printf("4.Upload Files\n");
    printf("5.Downlaod Files\n");

    scanf("%d",&choice );
    switch(choice)
    {
        case 1: //loginuser
          ;
          char *buffer;
          buffer = loginuser();
          printf("Value before writing  buffer is:%s\n",buffer );
          n = write(sockfd,buffer,strlen(buffer));
          if(n<0){
            error("ERROR writing to socket");
          }
          bzero(buffer,256);
          n = read(sockfd,buffer,255);
          if(n<0){
            error("ERROR reading from socket");
          }
          if(!strcmp(buffer,"1")){
            printf("Client is successfully authenticated at server end.\n");
          }
          else{
            printf("Client cannot be authenticated at server end.\n" );
          }


          case 2: //lets signup first
          {
            char *buffer;
            buffer = signupuser();;
            printf("signupuser():Value before writing buffer is:%s\n",buffer );
            n = write(sockfd,buffer,strlen(buffer));
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
          }


    }


  }while(choice!=0);
  //printf("%s\n",buffer);
  return 0;
}
