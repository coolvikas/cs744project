 #include <pthread.h>
 #include <stdio.h>
 #define NUM_THREADS     5



void loginuser(int,char[],char []);

 void *new_client_thread(void *threadid)
 {
    long tid;
    tid = (long)threadid;
    //printf("Hello World! It's me, thread #%ld!\n", tid);
    void sigint_handler(int sig); /* prototype */
    
    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0; // or SA_RESTART
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    int portno ,n;

    struct sockaddr_in server_addr;
    struct hostent *server;

    
    portno = atoi("6500");
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
    cout<<"before calling loginuser()"<<endl;
    loginuser(sockfd,uname,uname);
    cout<<"after calling loginuser()"<<endl;
    pthread_exit(NULL);
 }

 void spawn_clients(void)
 {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;
    for(t = 0; t<NUM_THREADS; t++){
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