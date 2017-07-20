#include<stdio.h>

#include<arpa/inet.h>

#include<netdb.h>
#include<unistd.h>
#include<stdlib.h>
int main(int argc,char* argv[])
{
struct hostent *host;
struct sockaddr_in server_addr;
int sock_des,port;
if(argc != 2)
{
printf("Usage ;./a.out hostname \n");
exit(1);
}
if ((host=gethostbyname(argv[1])) == NULL)
{
printf("Unknown Host\n");
exit(1);
}
for(port = 0; port<= 65535; port++)
{
if((sock_des = socket(AF_INET,SOCK_STREAM,0) == -1))
{
perror("socket");
exit(1);
}
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(port);
server_addr.sin_addr = *((struct in_addr *)host->h_addr);
if (connect(sock_des, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))!= -1)
{
printf("port %d is open \n",port);
}
close(sock_des);
}
return 0 ;
}
