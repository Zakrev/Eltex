#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *func(void *arg)
{
        int *id = (int *)arg;
        
        sleep(1);
        printf("%d\n", id[0]);
        pthread_exit(0);
}

int main()
{
        pthread_t tid;
        int id[5];
        int i;
        
        for(i = 0; i < 5; i++){
                id[i] = i;
                pthread_create(&tid, NULL, func, id + i);
                pthread_join(tid, 0);
        }
        printf("Done!");
        
        return 0;
}