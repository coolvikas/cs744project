#include <stdio.h>

#include <pthread.h>





int main(int argc, char **argv)
{
    int s;
    void *res;
    pthread_t t1;

    s = pthread_create(&t1, NULL, thread_func, "Hello World\n");

    int k = pthread_join(t1);

    if (s != 0)
        printf("Err\n");

    printf("In Main()\n");

    //s = pthread_detach(t1);

    if (s != 0)
        printf("Err\n");

    return 0;
}

void *thread_func(void *arg)
{
    char *s = (char *)arg;
    printf("%s", s);
    pthread_exit(0);
}