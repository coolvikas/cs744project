#include <fstream>
#include <stdio.h>
#include <string.h>
int main(int argc, char const *argv[])
{
	/* code */

		FILE *fptr;
      	//cout<<"before open file"<<endl;
  		fptr = fopen("new.txt","a");
      	//cout<<"after open"<<endl;
   
  		char ch[]="\n";
  		char empty[] = " ";
  		for(int i=1;i<=1000;i++){
  			char uname[20] ;
  			sprintf(uname,"%d",i);
  			fwrite(uname,strlen(uname),1,fptr); // EACH ELEMENT IS OF SIZE 1 BYTE TO BE WRITTEN AND THERE ARE SIZEOF(BUFFER) ELEMENTS
    		fwrite(empty,strlen(empty),1,fptr);
    		fwrite(uname,strlen(uname),1,fptr);
    		fwrite(ch,strlen(ch),1,fptr);
  	
  		}
  		fclose(fptr);
	return 0;
}

