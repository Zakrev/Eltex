#include "inet.h"
#include <pthread.h>

#define SADDR struct sockaddr_in
#define SADDR_SIZE sizeof(SADDR)
/*
        START_PORT - порт с которого начинать поиск
        MAX_RERTY - число попыток. Сколько портов можно перебрать для подключения.
*/
#define START_PORT 1130
#define MAX_RERTY 1000
/*
        PTINF - структура потока
        
        pth_d - дескриптор потока
        sock_d - дескриптор сокета
        srv - адрес сокета (индивидуальный сокет для потока)
        clients - кол-во обрабатываемых клиентов
        nclients - кол-во новых клиентов в очереди cqueu
        addr[] - структуры адресов клиентов (т.е. клиенты)
        cqueu - id структур новых клиентов (клиенты ждут, пока поток потвердит подключение)
        mutex - указатель на mutexs[n] из SINF, где n - id потока. Это индивидуальный мютекс потока.
        mfl - флаг на удаление потока
        
        MAX_PTH_CLIENTS - максимум клиентов для одного потока
*/
#define PTINF struct mPthreadInfo
#define MAX_PTH_CLIENTS 50

PTINF {
        pthread_t pth_d;
        int sock_d;
        int clients;
        int nclients;
        int mfl;
        SADDR srv;
        SADDR **addr;
        int cqueu[MAX_PTH_CLIENTS];
        pthread_mutex_t *mutex;
};

/*
        SINF - структура сервера
        
        sock_d - дескриптор сокета
        srv - адрес сокета сервера
        pinfs[] - структуры работающих потоков (NULL - свободная ячейка)
        mutexs[] - мютексы для каждого потока
        pinfs_ed - мютекс для списка потоков pinfs[]
        init - Менеджер новых подключений (клиентов). Поток для "встречи" новых клиентов
        manager - Менеджер работающих потоков. Поток для контролирования pinfs[] и вывода информации
        
        MAX_PTH - максимум потоков
        MANAGE_TIMEOUT - таймаут вывода информации и проверки потоков
*/
#define SINF struct mSystemInfo
#define MAX_PTH 500
#define MANAGE_TIMEOUT 1

SINF {
        int sock_d;
        SADDR srv;
        PTINF **pinfs;
        pthread_mutex_t mutexs[MAX_PTH];
        pthread_mutex_t pinfs_ed;
        pthread_t init;
        pthread_t manager;
};


SINF *InitSINF();
int StartServices(SINF *sinf);
void RemoveSINF(SINF *sinf);
PTINF *InitPTINF();
void RemovePTINF(PTINF *pinf);
int CreatePTH(SINF *sinf);
void *WorkingPTH(void *arg);
void ReadMSG(PTINF *pinf, MSG msg);
void CheckNewClients(PTINF *pinf);
int AddClient(SINF *sinf, SADDR addr);
void RemoveClient(PTINF *pinf, int id);
void CopySADDR(SADDR from, SADDR *to);
void *ManagerWorking(void *arg);
void *InitialConnect(void *arg);
