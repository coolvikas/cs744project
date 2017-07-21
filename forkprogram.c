#include<stdio.h>
#include<stdlib.h>
#define NFORKS 10
int i=0;
void do_nothing(int *x){
printf("%d\n",*x );
*x=*x+1;
printf("%d\n",*x );
}

int main(int argc, char *argv[]){
  int pid,j,status;

  for(j=0;j<NFORKS;j++){

    if((pid=fork())<0){
      printf("fork failed with error code = %d\n", pid );
      exit(0);
    }

    else if(pid == 0){
      do_nothing(&i);
      exit(0);
    }
    else{
      waitpid(pid,status,0);
    }
  }
  printf("i=%d\n",i );

}
