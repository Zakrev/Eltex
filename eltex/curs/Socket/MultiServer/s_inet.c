#include "s_inet.h"

/*
        Инициализируем главную структуру                
*/
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

/*
        Запускаем менеджер новых подключений и именеджер потоков
*/
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
/*
        Удаляем всю структуру сервера
*/
void RemoveSINF(SINF *sinf)
{
        int i;
        
        pthread_cancel(sinf->init);
        pthread_cancel(sinf->manager);
        shutdown(sinf->sock_d, SHUT_RDWR);
        close(sinf->sock_d);
        if(pthread_mutex_lock(&sinf->pinfs_ed) == 0){
                for(i = 0; i < MAX_PTH; i++){
                        if(pthread_mutex_lock(&sinf->mutexs[i]) == 0){
                                RemovePTINF(sinf->pinfs[i]);
                                sinf->pinfs[i] = NULL;
                                pthread_mutex_unlock(&sinf->mutexs[i]);
                        }
                }
                free(sinf->pinfs);
                pthread_mutex_unlock(&sinf->pinfs_ed);
        }
        free(sinf);    
}
/*
        Инициализируем структуру одного потока, обрабатывающего MAX_PTH_CLIENTS клиентов
*/
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
                Перебираем порты от START_PORT + MAX_RERTY до START_PORT
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
        pinf->mfl = MFL_JOB;
        pinf->mfl_type = MFL_JOB;
        pinf->addr = addr;
        pinf->sock_d = sock_d;
        pinf->clients = 0;
        pinf->nclients = 0;
        
        return pinf;
}

/*
        Удаляем структуру потока и всех его клиентов
        Для вызова нужно заблокировать мютекс
        pthread_mutex_lock(&sinf->mutexs[n]);
*/
void RemovePTINF(PTINF *pinf)
{
        if(pinf == NULL)
                return;
        
        int i;
        pthread_t pth_d = pinf->pth_d;
        
        if(pthread_cancel(pth_d) < 0)
                perror("Error on remove pthread"); 
        close(pinf->sock_d);
        for(i = 0; i < MAX_PTH_CLIENTS; i++){
                if(pinf->addr[i] == NULL)
                        continue;
                free(pinf->addr[i]);
                pinf->addr[i] = NULL;
        }
        free(pinf->addr);
        free(pinf);        
}
/*
        Создаем новый пустой поток для обработки клиентов
        Поток ставится в первую свободную ячейку sinf->pinfs
*/
int CreatePTH(SINF *sinf)
{
        int id;
        int rez = -1;
        
        if( pthread_mutex_lock(&sinf->pinfs_ed) == 0){               
                for(id = 0; id < MAX_PTH; id++){
                        if(sinf->pinfs[id] == NULL)
                                break;
                }
                if(id < MAX_PTH){
                        if( pthread_mutex_lock(&sinf->mutexs[id]) == 0){                       
                                sinf->pinfs[id] = InitPTINF();
                                if(sinf->pinfs[id] != NULL){
                                        if(pthread_create(&sinf->pinfs[id]->pth_d, NULL, WorkingPTH, sinf->pinfs[id]) < 0){
                                                perror("Error create pthread");
                                                RemovePTINF(sinf->pinfs[id]);
                                                sinf->pinfs[id] = NULL;     
                                        } else {
                                                rez = 0;
                                                sinf->pinfs[id]->mutex = &sinf->mutexs[id];
                                        }
                                }
                                pthread_mutex_unlock(&sinf->mutexs[id]);
                         }
                }                
                pthread_mutex_unlock(&sinf->pinfs_ed);
        }
        
        return rez;                
}
/*
        Поток для работы с клиентами
*/
void *WorkingPTH(void *arg)
{
        PTINF *pinf;
        MSG msg;
        int sock_d;
        struct pollfd pfd[1];
        int poll_rez;
        
        pinf = (PTINF *) arg;
        sock_d = pinf->sock_d;
        pfd[0].fd = sock_d;
        pfd[0].events = POLLIN;
        pfd[0].revents = 0;
        
        while(1){
                if( pthread_mutex_lock(pinf->mutex) < 0){
                        sleep(NCLIENTS_TIMEOUT);
                        
                        continue;
                }
                if(pinf->clients < 1){
                        CheckNewClients(pinf);
                        pthread_mutex_unlock(pinf->mutex);
                        sleep(NCLIENTS_TIMEOUT);
                        
                        continue;
                }
                pinf->mfl = pinf->mfl_type;
                pthread_mutex_unlock(pinf->mutex);
                
                poll_rez = poll(pfd, 1, NCLIENTS_TIMEOUT * 1000);
                if(poll_rez > 0){
                        if( recvfrom(sock_d, &msg, MSG_SIZE, MSG_DONTWAIT, NULL, NULL) < 0){
                                //perror("WorkingPTH (error recv)");
                        } else {
                                if(pthread_mutex_lock(pinf->mutex) == 0){
                                        ReadMSG(pinf, msg);
                                        pthread_mutex_unlock(pinf->mutex);
                                }
                        }
                } else {
                        if(poll_rez < 0)
                                perror("Error in poll");
                }
                
                if(pthread_mutex_trylock(pinf->mutex) == 0){
                        CheckNewClients(pinf);
                        pthread_mutex_unlock(pinf->mutex);
                }
        }
}

/*
        Обрабатывает входящий пакет
        Для вызова нужно заблокировать мютекс
        pthread_mutex_lock(pinf->mutex);
*/
void ReadMSG(PTINF *pinf, MSG msg)
{
        /*
                Эта функция обрабатывает сообщени. 
                Если клиент известный, то отправляем ему его-же сообщение и удаляем его.
                (При подключении, в функции CheckNewClients, сервер отправляет время)
        */
        socklen_t sl;
        SADDR addr;
        int sid = msg.sid;
        time_t now_time;

        if(sid < 0 || sid >= MAX_PTH_CLIENTS){
                printf("Unkown client: %d\n", sid);
                return;
        }
        sl = SADDR_SIZE;
        CopySADDR(*pinf->addr[sid], &addr);
        time(&now_time);
        //printf("%s\t%s(%d): %s\n", ctime(&now_time), inet_ntoa(addr.sin_addr), htons(addr.sin_port), msg.data_str);
        
        if(sendto(pinf->sock_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &addr, sl) < 0)
                perror("Error re-send");
        RemoveClient(pinf, sid);
}

/*
        Делает новых клиентов активными
        Для вызова нужно заблокировать мютекс
        pthread_mutex_lock(pinf->mutex);
*/
void CheckNewClients(PTINF *pinf)
{
        int i;
        MSG msg;
        socklen_t sl;
        time_t sec;
        struct tm *timeinfo;
                
        sl = SADDR_SIZE; 
        for(i = 0; pinf->nclients > 0 && i < MAX_PTH_CLIENTS; i++){
                if(pinf->cqueu[i] == -1)
                        continue;
                /*
                        Если id в списке есть, а структуры нет,
                        то просто убераем id из списка
                */
                if(pinf->addr[pinf->cqueu[i]] != NULL){
                        /*
                                Отправляем текущую дату клиенту.
                        */
                        
                        sec = time(NULL);
                        timeinfo = localtime(&sec);
                        sprintf(msg.data_str, "%s", asctime(timeinfo));
                        msg.sid = pinf->cqueu[i];         
                        if(sendto(pinf->sock_d, &msg, MSG_SIZE, MSG_DONTWAIT, (struct sockaddr *) pinf->addr[pinf->cqueu[i]], sl) < 0){
                                //perror("Error send (CheckNewClients)");
                        }
                        printf("%s\tNew client: %s (%d)\n", 
                                asctime(timeinfo), 
                                inet_ntoa(pinf->addr[pinf->cqueu[i]]->sin_addr),
                                ntohs(pinf->addr[pinf->cqueu[i]]->sin_port));
                        /*
                                Сразу удаляем клиента
                        */
                        RemoveClient(pinf, pinf->cqueu[i]);                       
                }
                pinf->cqueu[i] = -1;
                pinf->nclients--;
        }
}
/*
        Добавляет нового клиента
*/
int AddClient(SINF *sinf, SADDR c_addr)
{
        int i, j;
        int rez = -1;
        SADDR **addr;
                
        if( pthread_mutex_lock(&sinf->pinfs_ed) != 0)
                return -1;
        
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
                if(pthread_mutex_lock(&sinf->mutexs[i]) == 0){
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
                                                        rez = -2;
                                                        break;
                                                }
                                                /*
                                                        Клиент добавлен, выходим из всех циклов
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
        }
        pthread_mutex_unlock(&sinf->pinfs_ed);
        
        return rez;
}

/*
        Удаляет клиента из потока
        Для вызова нужно заблокировать мютекс
        pthread_mutex_lock(&sinf->mutexs[n]);
        или
        pthread_mutex_lock(pinf->mutex);
*/
void RemoveClient(PTINF *pinf, int id)
{
        if(pinf->addr[id] != NULL){
                free(pinf->addr[id]);
                pinf->addr[id] = NULL;
                pinf->clients--;
        }
}
/*
        Копирует данные из одной структуры адреса в другую
*/
void CopySADDR(SADDR from, SADDR *to)
{
        to->sin_family = from.sin_family;
        to->sin_port = from.sin_port;
        to->sin_addr.s_addr = from.sin_addr.s_addr;
}
/*
        Поток менеджера потоков
*/
void *ManagerWorking(void *arg)
{
        SINF *sinf;
        PTINF *pinf;
        int i;
        int max_clients = MAX_PTH_CLIENTS;
        char mfl;
        time_t now_time;

        sinf = (SINF *) arg;

        while(1){
                time(&now_time);
                if(pthread_mutex_lock(&sinf->pinfs_ed) == 0){
                        printf("%s\tPTH\tIP\t\tPORT\tClients (NEW\\ACT\\MAX)\n", ctime(&now_time));
                        for(i = 0; i < MAX_PTH; i++){
                                if(pthread_mutex_lock(&sinf->mutexs[i]) == 0){
                                        if(sinf->pinfs[i] != NULL){
                                                pinf = sinf->pinfs[i];
                                                /*
                                                        Если поток не используется, помечаем на удаление.
                                                        Если метка уже стоит, удаляем.
                                                */
                                                if(pinf->nclients > 0)
                                                        mfl = '+';
                                                else
                                                        mfl = ' ';
                                                switch(pinf->mfl){
                                                        case MFL_JOB:
                                                                        sinf->pinfs[i]->mfl = MFL_WAIT;
                                                        break;
                                                        case MFL_WAIT:
                                                                        sinf->pinfs[i]->mfl = MFL_KILL;
                                                                        mfl = '*';
                                                        break;
                                                        case MFL_KILL:
                                                                        RemovePTINF(pinf);
                                                                        sinf->pinfs[i] = NULL;
                                                        break;
                                                }
                                        }
                                        if(sinf->pinfs[i] != NULL){
                                                /*
                                                        Вывод информации о потоке
                                                */
                                                printf("\t%c%d\t%s\t%d\t%d\\%d\\%d\n", 
                                                mfl,
                                                i, 
                                                inet_ntoa(pinf->srv.sin_addr), 
                                                ntohs(pinf->srv.sin_port),
                                                pinf->nclients,
                                                pinf->clients - pinf->nclients,
                                                max_clients);
                                        }
                                        pthread_mutex_unlock(&sinf->mutexs[i]);
                                }
                        }
                        pthread_mutex_unlock(&sinf->pinfs_ed);
                }
                sleep(MANAGE_TIMEOUT);               
        }
}

/*
        Поток менеджера новых клиентов
        Функция будет пытаться подключить клиента пока не закончатся места в потоках и сами потоки
*/
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
                                Подключаем клиента
                        */
                        rez = AddClient(sinf, client);
                        /*
                                Исправляем ошибки
                        */
                        while( rez < 0){
                                switch(rez){
                                        case -1:
                                                if(CreatePTH(sinf) < 0)
                                                        /*
                                                                Отмена подключения
                                                        */
                                                        rez = 1;
                                                else {
                                                        /*
                                                                Попытка подключения
                                                        */
                                                        rez = AddClient(sinf, client);
                                                }
                                        break;
                                        case -2:
                                                /*
                                                        Отмена подключения
                                                */
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
