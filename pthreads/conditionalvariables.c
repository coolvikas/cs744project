#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

int done=0;
// volatile int done =0;
pthread_mutex_t m =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

void thr_exit(){
  pthread_mutex_lock(&m);
  done = 1;
  pthread_cond_signal(&c);
  pthread_mutex_unlock(&m);
}

void *child(void *arg){
  printf("child\n" );
  //done=1;
  thr_exit();
  return NULL;
}

void thr_join(){
  pthread_mutex_lock(&m);
  while(done == 0){
    pthread_cond_wait(&c,&m);
  }
  pthread_mutex_unlock(&m);
}
int main(int argc, char *argv[]){
  printf("parent:begin\n" );
  pthread_t p;
  pthread_create(&p, NULL, child, NULL);
  thr_join();
  //sleep(0);
  //while(done == 0);
  printf("parent:end\n" );
  return 0;
}
