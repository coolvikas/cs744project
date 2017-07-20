//creating multiple threads using pthread create functions and using them on the global variable i ;
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#define NTHREADS 5
int i=0;
void *do_nothing(void *null){
  i=i+1;
  pthread_exit(NULL);
}

int main(int argc, int *argv[]){
  int rc,i,j,detachstate;
  i=i+1;
  pthread_t tid;
  for(j=0;j<NTHREADS;j++){
    rc=pthread_create(&tid,NULL,do_nothing,NULL);
    if(rc){
      printf("ERROR;return code from pthread_create() is %d\n",rc );
      exit(-1);
    }
    printf("i = %d\n",i);

    }
  printf("%d\n",i );
  pthread_exit(NULL);
}
