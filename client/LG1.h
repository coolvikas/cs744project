#include <pthread.h>
#include <stdio.h>
#include <iostream>
using namespace std;
#define NUM_THREADS     1



int loginuser(int,char[],char []);
void logout(int ,int );
int upload(int ,char [],int );

 void *new_client_thread(void *threadid)
 {
    long tid;
    tid = (long)threadid;
   
    int portno ,n;

    struct sockaddr_in server_addr;
    struct hostent *server;

    
    portno = atoi("6702");
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
      perror("Error opening socket");
    }
    else
      printf("\n");

    server = gethostbyname("127.0.0.1");
    if(server == NULL){
      fprintf(stderr, "ERROR no such host\n" );
      exit(0);
    }
    bzero((char *)&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);
    server_addr.sin_port = htons(portno);
    if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
      perror("ERROR connecting");
    }
    char uname[20];
    sprintf(uname,"%ld",tid);
    //cout<<"before calling loginuser()"<<endl;
    int sessionid = loginuser(sockfd,uname,uname);
    cout<<"after calling loginuser()"<<endl;
    //sleep(1);
    
    char filename[20];
    sprintf(filename,"%s","image.jpg");
    int x = upload(sockfd,filename,sessionid);
    logout(sockfd,sessionid);
    
    pthread_exit(NULL);
 }

 void spawn_clients(void)
 {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;
    for(t = 1; t<=NUM_THREADS; t++){
       printf("In main: creating thread %ld\n", t);
       rc = pthread_create(&threads[t], NULL, new_client_thread, (void *)t);
       if (rc){
          printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
       }
       if ( ! pthread_detach(threads[t]) )
            printf("Thread detached successfully !!!\n");
    
    }

    /* Last thing that main() should do */
   pthread_exit(NULL);
 }