#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#define MAX 10
#define NUM_THREADS 9;
int buffer[MAX];
int fill_ptr =0;
int use_ptr = 0;
int count =0;

void put(int value){
  buffer[fill_ptr]= value;
  fill_ptr = (fill_ptr+1)%MAX;
  count ++;
}

int get (){
  int tmp = buffer[use_ptr];
  use_ptr = (use_ptr+1)%MAX;
  count -- ;
  return tmp;
}

pthread_cond_t empty, fill=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void *producer(void *arg){
  int i;
  long loops =(long)arg;
  printf("loops in producer = %lu\n",loops );
  for(i=0;i<loops;i++){
    printf("i in producer=%d\n",i );
    pthread_mutex_lock(&mutex);
    while (count == MAX) {
      pthread_cond_wait(&empty,&mutex);
      /* code */
    }
    put(i);
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
  }
}

void *consumer(void *arg){
  int i;
    long loops =(long)arg;
    printf("loops in consumer = %lu\n",loops );
  for(i=0;i<loops;i++){
    printf("i in consumer=%d\n",i );
    pthread_mutex_lock(&mutex);
    while (count == 0) {
      pthread_cond_wait(&fill, &mutex);
      /* code */
    }
    int tmp = get();
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    printf("%d\n",tmp );
    pthread_exit(NULL);
  }
}

int main(int argc, int *argv[]){
  long loops = 2;
  printf("parent:begin\n" );
  pthread_t p;
  pthread_create(&p, NULL, producer, (void *)loops);
  pthread_t c;
  pthread_create(&c,NULL,consumer,(void *)loops);

  pthread_exit(NULL);
}
