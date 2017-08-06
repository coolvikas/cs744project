#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h> //for read and write functions

void error(char *msg){
  perror(msg);
  exit(1);
}

int checkcredentials(char uname[20],char passwd[20]){
  FILE *fptr;
  char funame[20],fpasswd[20];
  char line[30];
  int flag =0;
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
  fclose(fptr);
  return flag;
}

int signupuser(char uname[20],char passwd[20]){

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

  fclose(fptr);
  return signupdone;
}


int main(int argc, char *argv[]){

  int sockfd, newsockfd, portno, clilen,x;
  char buffer[256];
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
  newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
  if(newsockfd < 0){
    error("ERROR on accept");
  }
  bzero(buffer,256);
  //char c[500];

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
      char uname[20],passwd[20];
      int initial;
      sscanf(buffer,"%d %s %s",&initial,uname,passwd);
      printf("Received username passwd from client is:\n" );
      fputs(uname,stdout);
      fputs(" ",stdout);
      fputs(passwd,stdout);
      printf("\n" );
      int flag;
      flag = checkcredentials(uname,passwd);
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
    }

    case 2:
    {
      char uname[20],passwd[20];
      int initial;
      sscanf(buffer,"%d %s %s",&initial,uname,passwd);
      printf("Received username passwd from client is:\n" );
      fputs(uname,stdout);
      fputs(" ",stdout);
      fputs(passwd,stdout);
      printf("\n" );
      int flag;
      flag = signupuser(uname,passwd);
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


    }



  }

  close(newsockfd);
  close(sockfd);

  /*if(flag == 1){

  } */

  return 0;
}
