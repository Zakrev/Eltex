#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *func(void *arg)
{
        printf("%s\n", (char *)arg);
        sleep(1);
        pthread_exit(0);
}

int main()
{
        pthread_t tid;
        char *str = "Hello!";
        
        pthread_create(&tid, NULL, func, str);
        pthread_join(tid, 0);
        printf("Done!");
        
        return 0;
}