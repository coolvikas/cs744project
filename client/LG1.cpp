 #include <pthread.h>
 #include <stdio.h>

 #define NUM_THREADS     5

 void *new_client_thread(void *threadid)
 {
    long tid;
    tid = (long)threadid;
    printf("Hello World! It's me, thread #%ld!\n", tid);
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
    }

    /* Last thing that main() should do */
    pthread_exit(NULL);
 }