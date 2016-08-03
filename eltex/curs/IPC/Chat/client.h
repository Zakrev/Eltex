#include "chat.h"

#define SINFO struct mServerInfo

/*
        clients - кол-во клиентов. Максимум SERVER_T - 1
        names - таблица имен
        ids - массив id. 0 - свободен, 1 - занят
        q_serv, q_msg - дескрипторы очередей
        p_serv, p_msg - дескрипторы потоков
        m_sinfo - мютекс для чтения/записи names, ids, clients
        m_id - мютекс для чтения/записи id
        name - имя клиента
        id - id клиента
*/

SINFO {
        int id;
        int clients;
        char names[SERVER_T][STR_SIZE];
        int ids[SERVER_T];
        int q_serv;
        int q_msg;
        pthread_t p_serv;
        pthread_t p_msg;
        pthread_mutex_t m_sinfo;
        pthread_mutex_t m_id;
        char name[STR_SIZE];
};

SINFO *InitClient();
int InitPthread(SINFO *inf);
void CloseClient(SINFO *inf);
void *TrackingServerQueue(void *arg);
void *TrackingMessageQueue(void *arg);
int SetIdStatus(SINFO *inf, int id, int stat);
int GetIdStatus(SINFO *inf, int id, int *stat);
int AddClient(SINFO *inf, char *cname, int id);
int EraseClient(SINFO *inf, int id);
int SendMSG(SINFO *inf, MSG *msg);
int RecvMSG(SINFO *inf, MSG *msg, int type);
int SendSMSG(SINFO *inf, SMSG *msg);
int RecvSMSG(SINFO *infm, SMSG *msg, int type);
int GetClientID(SINFO *inf, char *cname);
int GetClientName(SINFO *inf, int id, char *buff);
int NewName(char *buff);
