#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define SKLAD struct MarketSklad
#define CLIENT struct MarketClient
#define SCOUNT 5
#define CLIENT_COUNT 10
#define SMAX 200

SKLAD {
        int id;
        int trash;
        pthread_mutex_t mutex;
};

CLIENT {
        int id;
        int trash;
        SKLAD **market;
        pthread_t p_id;
};

int GetRand(int a, int b)
{
        return rand() % (b - a + 1) + a;
}

SKLAD *InitSKLAD(int id)
{
        SKLAD *sklad;
        
        sklad = malloc(sizeof(SKLAD));
        if(sklad == NULL)
                return NULL;
        sklad->id = id;
        sklad->trash = 0;
        pthread_mutex_init(&sklad->mutex, NULL);
        
        return sklad;
}

CLIENT *InitCLIENT(int id, SKLAD **market)
{
        CLIENT *client;
        
        client = malloc(sizeof(CLIENT));
        if(client == NULL)
                return NULL;
        client->id = id;
        client->trash = GetRand(1, 500);
        client->market = market;
        
        return client;
}

void ChangeTrash(SKLAD *sklad, int val, int *push)
{
        pthread_mutex_lock(&sklad->mutex);
        *push = sklad->trash + val;
        
        if(*push > SMAX){
                sklad->trash = SMAX;
                *push = val - (*push - SMAX);
        } else {
                if(*push < 0){
                        *push = -sklad->trash;
                        sklad->trash = 0;
                } else {
                        sklad->trash += val;
                        *push = val;
                }
        }
        
        pthread_mutex_unlock(&sklad->mutex);
}

void ChangeTrash_try(SKLAD *sklad, int val, int *push)
{
        if( pthread_mutex_trylock(&sklad->mutex) == EBUSY )
                return;
                
        *push = sklad->trash + val;
        
        if(*push > SMAX){
                sklad->trash = SMAX;
                *push = val - (*push - SMAX);
        } else {
                if(*push < 0){
                        *push = -sklad->trash;
                        sklad->trash = 0;
                } else {
                        sklad->trash += val;
                        *push = val;
                }
        }
        
        pthread_mutex_unlock(&sklad->mutex);
}

void *FillMarket(void *arg)
{
        SKLAD **market = (SKLAD **)arg;
        int i;
        int trash;
        int push;
        int push_count;
        
        while(1){
                trash = 900;
                for(i = 0, push_count = 0; push_count < SCOUNT && trash > 0; i++){
                        if(i == SCOUNT)
                                i = 0;
                        push = 0;
                        ChangeTrash_try(market[i], GetRand(0, trash), &push);
                        if( push != 0 ){
                                trash -= push;
                                push_count++;
                                printf("Loader: push %d to Sklad %d. Summ %d\n", push, market[i]->id, market[i]->trash);
                        }
                }
                printf("Loader: not loaded %d\n", trash);
                sleep(5);
       }
        
        pthread_exit(0);
}

void *LiveClient(void *arg)
{
        CLIENT *my = (CLIENT*)arg;
        
        int i, s_id;
        int push;
        
        while(my->trash > 0){
                for(i = 0; i < 10; i++){
                        s_id = GetRand(0, SCOUNT - 1);
                        push = 0;
                        ChangeTrash_try(my->market[s_id], -GetRand(1, 100), &push);
                        my->trash += push;
                        printf("Client(%d): pop %d from Sklad %d(%d)\n", my->id, -push, s_id + 1, my->market[s_id]->trash);
                        sleep(GetRand(0, 5));
                }
        }
        printf("Client(%d): out\n", my->id);
        
        pthread_exit(0);
}

int main()
{
        SKLAD **market;
        CLIENT **clients;
        int i;
        pthread_t loader;
        
        srand(getpid());
        market = malloc(sizeof(SKLAD*) * SCOUNT);
        for(i = 0; i < SCOUNT; i++)
                market[i] = InitSKLAD(i + 1);
        clients = malloc(sizeof(CLIENT*) * CLIENT_COUNT);
        for(i = 0; i < CLIENT_COUNT; i++){
                clients[i] = InitCLIENT(i + 1, market);
                pthread_create(&clients[i]->p_id, NULL, LiveClient, clients[i]);
        }
        pthread_create(&loader, NULL, FillMarket, market);
        for(i = 0; i < CLIENT_COUNT; i++)
                pthread_join(clients[i]->p_id, 0);
        pthread_cancel(loader);

        return 0;
}