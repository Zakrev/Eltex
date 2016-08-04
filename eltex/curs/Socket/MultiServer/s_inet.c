#include "s_inet.h"

SINF *InitSINF()
{
        SINF *sinf;
        PTINF **pinfs;
        SADDR srv;
        int sock_d;
        int i;
        socklen_t sl;
        
        sock_d = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sock_d <= 0){
                perror("Error to create socket");
                return NULL;
        }
        srv.sin_family = AF_INET;
        srv.sin_port = htons(SERVER_PORT);
        if(inet_aton(SERVER_IP, &srv.sin_addr) < 0){
                printf("Erro ip "SERVER_IP"\n");
                goto exit1;
        }
        sl = SADDR_SIZE;
        if(bind(sock_d, (struct sockaddr *) &srv, sl) < 0){
                perror("Error of bind socket");
                goto exit1;
        }        
        pinfs = malloc(sizeof(PTINF *) * MAX_PTH);
        if(pinfs == NULL)
                goto exit1;

        sinf = malloc(sizeof(SINF));
        if(sinf == NULL)
                goto exit1;
        CopySADDR(srv, &sinf->srv);
        sinf->sock_d = sock_d;
        sinf->pinfs = pinfs;
        for(i = 0; i < MAX_PTH; i++)
        {
                sinf->pinfs[i] = NULL;
                pthread_mutex_init(&sinf->mutexs[i], 0);
        }
        pthread_mutex_init(&sinf->pinfs_ed, 0);
        
        return sinf;
        
        exit1:
                close(sock_d);
                return NULL;
}

int StartServices(SINF *sinf)
{
        if(pthread_create(&sinf->init, NULL, InitialConnect, sinf) < 0){
                perror("Error create InitialConnect");
                return -1;
        }
        if(pthread_create(&sinf->manager, NULL, ManagerWorking, sinf) < 0){
                pthread_cancel(sinf->init);
                perror("Error create ManagerWorking");
                return -1;
        }

        return 0;
}

void RemoveSINF(SINF *sinf)
{
        int i;
        
        pthread_cancel(sinf->init);
        pthread_cancel(sinf->manager);        
        shutdown(sinf->sock_d, SHUT_RDWR);
        close(sinf->sock_d);
        pthread_mutex_lock(&sinf->pinfs_ed);        
        for(i = 0; i < MAX_PTH; i++){
                RemovePTINF(sinf->pinfs[i]);
        }
        free(sinf->pinfs);
        pthread_mutex_unlock(&sinf->pinfs_ed);
        free(sinf);    
}

PTINF *InitPTINF()
{
        PTINF *pinf;
        SADDR **addr;
        int sock_d;
        int i;
        SADDR srv;
        socklen_t sl;
        int fl;
        
        sock_d = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sock_d <= 0){
                perror("Error to create socket");
                return NULL;
        }
        srv.sin_family = AF_INET;
        if(inet_aton(SERVER_IP, &srv.sin_addr) < 0){
                printf("Erro ip "SERVER_IP"\n");
                return NULL;
        }
        sl = SADDR_SIZE;
        /*
                Поиск свободного порта
        */
        fl = MAX_RERTY;
        while(fl >= 0){
                srv.sin_port = htons(START_PORT + fl);
                if(bind(sock_d, (struct sockaddr *) &srv, sl) < 0){
                        fl--;
                        continue;
                }
                fl = -5;                
        }       
        if(fl != -5){
                perror("Error of bind socket");
                return NULL;
        }     
        
        
        pinf = malloc(sizeof(PTINF));
        if(pinf == NULL){
                return NULL;
        }
        addr = malloc(sizeof(SADDR *) * MAX_PTH_CLIENTS);
        if(addr == NULL){
                free(pinf);
                return NULL;
        }
        for(i = 0; i < MAX_PTH_CLIENTS; i++){
                addr[i] = NULL;
                pinf->cqueu[i] = -1;
        }
        CopySADDR(srv, &pinf->srv);
        pinf->mfl = 0;
        pinf->addr = addr;
        pinf->sock_d = sock_d;
        pinf->clients = 0;
        pinf->nclients = 0;

        return pinf;
}

/*
        Не забыть pthread_mutex_lock(&sinf->pinfs_ed);
*/
void RemovePTINF(PTINF *pinf)
{
        if(pinf == NULL)
                return;
        
        int i;
        
        close(pinf->sock_d);
        for(i = 0; i < MAX_PTH_CLIENTS; i++){
                if(pinf->addr[i] != NULL){
                        pthread_cancel(pinf->pth_d);
                        free(pinf->addr[i]);
                }
        }
        free(pinf->addr);
        free(pinf);
}

int CreatePTH(SINF *sinf)
{
        int id;
        int rez = -1;
        
        pthread_mutex_lock(&sinf->pinfs_ed);
        for(id = 0; id < MAX_PTH; id++){
                if(sinf->pinfs[id] == NULL)
                        break;
        }
        if(id < MAX_PTH){
                sinf->pinfs[id] = InitPTINF();
                if(sinf->pinfs[id] != NULL){
                        if(pthread_create(&sinf->pinfs[id]->pth_d, NULL, WorkingPTH, sinf->pinfs[id]) < 0){
                                perror("Error create pthread");
                                RemovePTINF(sinf->pinfs[id]);      
                        } else {
                                rez = 0;
                                sinf->pinfs[id]->mutex = &sinf->mutexs[id];
                        }
                }   
        }
        pthread_mutex_unlock(&sinf->pinfs_ed);
        
        return rez;                
}

void *WorkingPTH(void *arg)
{
        PTINF *pinf;
        MSG msg;
        
        pinf = (PTINF *) arg;
        while(1){
                if( recvfrom(pinf->sock_d, &msg, MSG_SIZE, MSG_DONTWAIT, NULL, NULL) < 0){
                        //perror("WorkingPTH (error recv)");
                } else {
                        ReadMSG(pinf, msg);
                }
                CheckNewClients(pinf);
        }
}

void ReadMSG(PTINF *pinf, MSG msg)
{
        socklen_t sl;
        SADDR addr;
        int sid = msg.sid;

        if(sid < 0 || sid >= MAX_PTH_CLIENTS){
                printf("Unkown client: %d\n", sid);
                return;
        }
        sl = SADDR_SIZE;
        CopySADDR(*pinf->addr[sid], &addr);
        //printf("New message from %s(%d): %s\n", inet_ntoa(addr.sin_addr), htons(addr.sin_port), msg.data_str);

        if(sendto(pinf->sock_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &addr, sl) < 0)
                perror("Error re-send");
        //sleep(5);
        RemoveClient(pinf, sid);
}

void CheckNewClients(PTINF *pinf)
{
        int i;
        MSG msg;
        socklen_t sl;
        time_t sec;
        struct tm *timeinfo;
                
        sl = SADDR_SIZE;       
        pthread_mutex_lock(pinf->mutex);
        for(i = 0; i < MAX_PTH_CLIENTS; i++){
                if(pinf->cqueu[i] == -1)
                        break;
                /*
                        Если id в списке есть, а структуры нет,
                        то просто убераем id из списка
                */
                if(pinf->addr[pinf->cqueu[i]] != NULL){
                        sec = time(NULL);
                        timeinfo = localtime(&sec);
                        sprintf(msg.data_str, "%s", asctime(timeinfo));
                        msg.sid = pinf->cqueu[i];
                                     
                        if(sendto(pinf->sock_d, &msg, MSG_SIZE, MSG_DONTWAIT, (struct sockaddr *) pinf->addr[pinf->cqueu[i]], sl) < 0){
                                //perror("Error send (CheckNewClients)");
                        }                       
                }
                pinf->cqueu[i] = -1;
                pinf->nclients--;
        }
        pthread_mutex_unlock(pinf->mutex);
}

int AddClient(SINF *sinf, SADDR c_addr)
{
        int i, j;
        int rez = -1;
        SADDR **addr;
                
        pthread_mutex_lock(&sinf->pinfs_ed);
        for(i = 0; i < MAX_PTH; i++){
                /*
                        Если нет созданных pinfs
                        вернет -1
                */
                if(sinf->pinfs[i] == NULL)
                        continue;
                /*
                        Если нет свободных pinf
                        вернет -1
                */
                pthread_mutex_lock(&sinf->mutexs[i]);
                if(sinf->pinfs[i]->clients < MAX_PTH_CLIENTS){
                        addr = sinf->pinfs[i]->addr;
                        for(j = 0; j < MAX_PTH_CLIENTS; j++){
                                if(addr[j] == NULL){
                                        addr[j] = malloc(sizeof(SADDR));
                                        if(addr[j] == NULL){
                                                /*
                                                        Не получилось выделить память
                                                        вернет -2
                                                */
                                                break;
                                        }
                                        /*
                                                Клиент добавлен, выходим из циклов
                                        */
                                        sinf->pinfs[i]->cqueu[sinf->pinfs[i]->nclients] = j;
                                        sinf->pinfs[i]->nclients++;
                                        sinf->pinfs[i]->clients++;
                                        CopySADDR(c_addr, addr[j]);
                                        rez = 0;
                                        break;
                                }
                        }
                        
                        pthread_mutex_unlock(&sinf->mutexs[i]);
                        break;
                }
                pthread_mutex_unlock(&sinf->mutexs[i]);
        }
        pthread_mutex_unlock(&sinf->pinfs_ed);
        
        return rez;
}

void RemoveClient(PTINF *pinf, int id)
{
        pthread_mutex_lock(pinf->mutex);
        if(pinf->addr[id] != NULL){
                free(pinf->addr[id]);
                pinf->addr[id] = NULL;
                pinf->clients--;
        }
        pthread_mutex_unlock(pinf->mutex);
}

void CopySADDR(SADDR from, SADDR *to)
{
        to->sin_family = from.sin_family;
        to->sin_port = from.sin_port;
        to->sin_addr.s_addr = from.sin_addr.s_addr;
}

void *ManagerWorking(void *arg)
{
        SINF *sinf;
        int i;
        int max_clients = MAX_PTH_CLIENTS;
        char mfl;

        sinf = (SINF *) arg;

        while(1){  
                pthread_mutex_lock(&sinf->pinfs_ed);
                printf("Server information:\n\tPTH\tIP\t\tPORT\tClients (NEW\\ACT\\MAX)\n");
                        for(i = 0; i < MAX_PTH; i++){
                                if(sinf->pinfs[i] == NULL)
                                        continue;
                                /*
                                        Если поток не используется, помечаем на удаление.
                                        Если метка уже стоит, удаляем.
                                */
                                mfl = ' ';
                                switch(sinf->pinfs[i]->mfl){
                                        case 0:
                                                if(sinf->pinfs[i]->clients == 0 && sinf->pinfs[i]->nclients == 0){
                                                        sinf->pinfs[i]->mfl = 1;
                                                        mfl = '*';
                                                }
                                        break;
                                        case 1:
                                                if(sinf->pinfs[i]->clients == 0 && sinf->pinfs[i]->nclients == 0){
                                                        RemovePTINF(sinf->pinfs[i]);
                                                        sinf->pinfs[i] = NULL;
                                                } else {
                                                        sinf->pinfs[i]->mfl = 0;                                                        
                                                }
                                        break;
                                }
                                if(sinf->pinfs[i] == NULL)
                                        continue;
                                printf("\t%d%c\t%s\t%d\t%d\\%d\\%d\n", 
                                i,
                                mfl, 
                                inet_ntoa(sinf->pinfs[i]->srv.sin_addr), 
                                ntohs(sinf->pinfs[i]->srv.sin_port),
                                sinf->pinfs[i]->nclients,
                                sinf->pinfs[i]->clients - sinf->pinfs[i]->nclients,
                                max_clients);
                        }
                pthread_mutex_unlock(&sinf->pinfs_ed);
                sleep(MANAGE_TIMEOUT);               
        }
}

void *InitialConnect(void *arg)
{
        SINF *sinf;
        SADDR client;
        socklen_t sl;
        MSG msg;
        int rez;

        sinf = (SINF *) arg;
        sl = SADDR_SIZE;
        while(1){
                if(recvfrom(sinf->sock_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &client, &sl) < 0){
                        perror("InitialConnect (error recv)");
                        sleep(5);
                } else {
                        /*
                                Подключаем
                        */
                        rez = AddClient(sinf, client);
                        /*
                                Исправляем ошибки
                        */
                        while( rez < 0){
                                switch(rez){
                                        case -1:
                                                if(CreatePTH(sinf) < 0)
                                                        rez = 1;
                                                else {
                                                        rez = AddClient(sinf, client);
                                                }
                                        break;
                                        case -2:
                                                rez = 2;
                                        break;
                                }
                        }
                        /*
                                Результат
                        */
                        switch(rez){
                                case 1:
                                        printf("InitialConnect (error): Client don't connect. Error of create new PTH.\n");
                                break;
                                case 2:
                                        printf("InitialConnect (error): Client don't connect. Error of create SADDR.\n");
                                break;
                        }
                }
        }
}
