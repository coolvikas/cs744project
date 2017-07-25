#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#define MAX 10
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

pthread_cond_t empty, fill;
pthread_mutex_t mutex;
void *producer(int *arg){
  int i;
  int loops =( *arg);
  for(i=0;i<loops;i++){
    pthread_mutex_lock(&mutex);
    while (count == MAX) {
      pthread_cond_wait(&empty,&mutex);
      /* code */
    }
    put(i);
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
  }
}

void *consumer(int *arg){
  int i;
  int loops = (*arg);
  for(i=0;i<loops;i++){
    pthread_mutex_lock(&mutex);
    while (count == 0) {
      pthread_cond_wait(&fill, &mutex);
      /* code */
    }
    int tmp = get();
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    printf("%d\n",tmp );
  }
}

int main(int argc, int *argv[]){
  int loops = 10;
  printf("parent:begin\n" );
  pthread_t p;
  pthread_create(&p, NULL, producer, &loops);
  pthread_t c;
  pthread_create(&c,NULL,consumer,&loops);


}
