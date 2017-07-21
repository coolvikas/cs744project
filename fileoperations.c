#include<stdio.h>
#include<stdlib.h>

int main(int argc, int *argv[]){
  FILE *fp;
  fp = fopen("test.txt","r");
  int c ,i;
  char line[30];
  char first=[20];
  char pswd=[20];
  char *input1 = "admin";
  while(fgets(line,sizeof(line),fp)!= NULL){
    sscanf(line,"%s%s",&first,&pswd);
    if(*first == input1){
      printf("Success\n");
    }
    //fputs(first,stdout);
    //printf("\t" );
    //fputs(pswd,stdout);
    //printf("\n" );
  }
  fclose(fp);
  return 0;
}
