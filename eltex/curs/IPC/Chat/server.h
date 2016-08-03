#include "chat.h"

#define SINFO struct mServerInfo

/*
        clients - кол-во клиентов. Максимум SERVER_T - 1
        names - таблица имен
        ids - массив id. 0 - свободен, 1 - занят, 2 - опрашивается
        q_serv, q_msg - дескрипторы очередей
        p_serv, p_msg, p_req - дескрипторы потоков
        m_sinfo - мютекс для чтения/записи SINFO
*/

SINFO {
        int clients;
        char names[SERVER_T][STR_SIZE];
        int ids[SERVER_T];
        int q_serv;
        int q_msg;
        pthread_t p_serv;
        pthread_t p_msg;
        pthread_t p_req;
        pthread_mutex_t m_sinfo;
};

SINFO *InitServer();
int InitPthread(SINFO *inf);
void CloseServer(SINFO *inf);
void *TrackingServerQueue(void *arg);
void *TrackingMessageQueue(void *arg);
void *TrackingRequestClient(void *arg);
int GetClientCount(SINFO *inf);
int SetIdStatus(SINFO *inf, int id, int stat);
int GetIdStatus(SINFO *inf, int id, int *stat);
int AddClient(SINFO *inf, char *cname);
int EraseClient(SINFO *inf, int id);
int SendMSG(SINFO *inf, MSG *msg);
int RecvMSG(SINFO *inf, MSG *msg, int type);
int SendBroadcastMSG(SINFO *inf, MSG *msg, int *erase, int e_len);
int SendSMSG(SINFO *inf, SMSG *msg);
int RecvSMSG(SINFO *infm, SMSG *msg, int type);
int GetClientID(SINFO *inf, char *cname);
int GetClientName(SINFO *inf, int id, char *buff);