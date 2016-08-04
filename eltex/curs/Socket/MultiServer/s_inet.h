#include "inet.h"
#include <pthread.h>

#define SADDR struct sockaddr_in
#define SADDR_SIZE sizeof(SADDR)
/*
        START_PORT - порт с которого начинать поиск
        NRERTY - число попыток. Сколько портов можно перебрать для подключения.
*/
#define START_PORT 1130
#define MAX_RERTY 1000
/*
        pth_d - дескриптор потока
        sock_d - дескриптор сокета
        srv - адрес сокета сервера (индивидуальный сокет для потока)
        clients - кол-во обрабатываемых клиентов
        nclients - кол-во новых клиентов в очереди cqueu
        addr[] - структуры адресов клиентов (т.е. клиент)
        cqueu - id структур новых клиентов
        mutex - указатель на mutexs[n] из SINF, где n - id потока
        mfl - флаг на удаление потока
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
        SINF
        sock_d - дескриптор сокета
        srv - адрес сокета сервера
        pinfs[] - структуры работающих потоков
        mutexs[] - мютексы для обращения к clients, new_clients в pinfs[n]
        pinfs_ed - мютекс для редактирования списка pinfs (удаление/создание)
                pinfs_ed нужен для менджера потоков. 
                Чтобы главный поток (создает, добавляет клиентов)
                 и менеджер(удаляет, читает информацию) не пересикались.
        init - поток для "встречи" новых клиентов
        manager - (менеджер)поток для контролирования pinfs и вывода информации о клиентах
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
