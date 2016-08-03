#include "server.h"

int main()
{
        SINFO *inf;
        
        inf = InitServer();
        
        if(inf->q_serv < 0 || inf->q_msg < 0){
                CloseServer(inf);
                return 1;
        }
        if(InitPthread(inf) != 0){
                CloseServer(inf);
                return 1;
        }
        
        scanf("e");
        
        CloseServer(inf);

        return 0;
}

SINFO *InitServer()
{
        SINFO *inf;
        int i;
        
        inf = malloc(sizeof(SINFO));
        if(inf == NULL){
                perror("Error create SINFO");
                return NULL;
        }
        inf->clients = 0;
        for(i = 0; i < SERVER_T; i++){
                memset(inf->names[i], '\0', STR_SIZE);
                inf->ids[i] = 0;
        }
        inf->q_serv = msgget(SERVER_QKEY, IPC_CREAT | 0666);
        if(inf->q_serv < 0)
                perror("Error create server queue");
        inf->q_msg = msgget(MESSAGE_QKEY, IPC_CREAT | 0666);
        if(inf->q_msg < 0)
                perror("Error create message queue");
        
        return inf;
}

int InitPthread(SINFO *inf)
{
        pthread_mutex_init(&inf->m_sinfo, NULL);
        if(pthread_create(&inf->p_serv, NULL, TrackingServerQueue, inf) < 0)
                return 1;
        if(pthread_create(&inf->p_msg, NULL, TrackingMessageQueue, inf) < 0)
                return 1;
        if(pthread_create(&inf->p_req, NULL, TrackingRequestClient, inf) < 0)
                return 1;
        
        return 0;
}

void CloseServer(SINFO *inf)
{
        if( inf->q_serv >= 0 ){
                msgctl(inf->q_serv, IPC_RMID, NULL);
        }
        if( inf->q_msg >= 0 ){
                msgctl(inf->q_msg, IPC_RMID, NULL);
        }
        pthread_cancel(inf->p_serv);
        pthread_cancel(inf->p_msg);
        pthread_cancel(inf->p_req);
        free(inf);
        
}

void *TrackingServerQueue(void *arg)
{
        SMSG smsg;
        MSG msg;
        SINFO *inf = (SINFO*)arg;
        char cname[STR_SIZE];
        int id;
        int erase[SERVER_T];
        int err;
        
        while(1){
                if( RecvSMSG(inf, &smsg, SERVER_T) == -1){
                        perror("Error on receive SMSG");
                        continue;
                }
                switch(smsg.msg_fl){
                        case RREG:
                                err = AddClient(inf, smsg.data_str);
                                if(err == 0){
                                        id = GetClientID(inf, smsg.data_str);
                                        if(id > 0){
                                                smsg.d_type = SERVER_T + 1;
                                                smsg.msg_fl = SREG;
                                                smsg.data_int = id;
                                                /*
                                                smsg.data_str - не меняем.
                                                */
                                                if( SendSMSG(inf, &smsg) != 0 ){
                                                        EraseClient(inf, id);
                                                        printf("RREG: %s not registered\n", smsg.data_str);
                                                } else {
                                                        printf("RREG: %s registered id(%d)\n", inf->names[id], id);
                                                        erase[0] = id;
                                                        msg.msg_fl = CNEW;
                                                        msg.data_int = id;
                                                        sprintf(msg.data_str_long, "%s", smsg.data_str);
                                                        SendBroadcastMSG(inf, &msg, erase, 1);
                                                        
                                                        break;
                                                }
                                        } else {
                                                printf("RREG: Error find client(%s)\n", smsg.data_str);
                                        }
                                }
                                smsg.d_type = SERVER_T + 1;
                                smsg.msg_fl = EREG;
                                smsg.data_int = err;
                                /*
                                smsg.data_str - не меняем.
                                */
                                SendSMSG(inf, &smsg);
                                printf("RREG: %s not registered. Error %d\n", smsg.data_str, err);
                        break;
                        case CEXIT:
                                id = smsg.data_int;
                                if(GetClientName(inf, id, cname) != 0){
                                        printf("CEXIT: Error of finding client(%d)\n", smsg.data_int);
                                        break;
                                }
                                printf("CEXIT: (%d)%s\n", smsg.data_int, cname);
                                EraseClient(inf, smsg.data_int);
                                msg.msg_fl = COUT;
                                msg.data_int = id;
                                SendBroadcastMSG(inf, &msg, NULL, 0);
                        break;
                        case CREQE:
                                printf("CREQE: %d is online\n", smsg.data_int);
                                SetIdStatus(inf, smsg.data_int, 1);
                        break;
                        default:
                                printf("Error on receive SMSG: Unknown msg_fl (%d)\n", smsg.msg_fl);
                        break;
                }
        }
        
        pthread_exit(0);
}

void *TrackingMessageQueue(void *arg)
{
        MSG msg;
        SINFO *inf = (SINFO *)arg;
        SINFO *inf2 = inf;
        /*
                Ошибка с указателем на inf
                при получении MSG inf затирается на первый символ поля data_str_long
        */
        int erase[SERVER_T];
        char cname[STR_SIZE];
        
        while(1){
                printf("%x - %x\n", inf, inf2);
                if( RecvMSG(inf, &msg, SERVER_T) == -1){
                        perror("Error on receive MSG");
                        continue;
                }
                switch(msg.msg_fl){
                        case MNEW:
                                if(GetClientName(inf, msg.data_int, cname)){
                                        printf("MNEW: Error of finding client (%d)\n", msg.data_int);
                                        printf("MNEW: Client (%d)\n\t%s\n", msg.data_int, msg.data_str_long);
                                } else
                                        printf("MNEW: %s\n\t%s\n", cname, msg.data_str_long);
                                erase[0] = msg.data_int;
                                msg.msg_fl = MNEWB;
                                SendBroadcastMSG(inf, &msg, erase, 1);
                        break;
                        default:
                                printf("Error on receive MSG: Unknown msg_fl (%d)\n", msg.msg_fl);
                        break;
                }
        }
        
        pthread_exit(0);
}

void *TrackingRequestClient(void *arg)
{
        SMSG smsg;
        SINFO *inf = (SINFO *)arg;
        int i;
        int id_stat;
        
        while(1){
                if( GetClientCount(inf) > 0){
                        smsg.msg_fl = CREQ;
                        for(i = 0; i < SERVER_T; i++){
                                GetIdStatus(inf, i, &id_stat);
                                switch(id_stat){
                                        case 1:
                                                smsg.d_type = i;
                                                SendSMSG(inf, &smsg);
                                                SetIdStatus(inf, i, 2);
                                        break;
                                        case 2:
                                                EraseClient(inf, i);
                                        break;
                                }
                        }
                }
                sleep(20);
        }
        
        pthread_exit(0);
}

int GetClientCount(SINFO *inf)
{
        int clients;
        
        pthread_mutex_lock(&inf->m_sinfo);
        clients = inf->clients;
        pthread_mutex_unlock(&inf->m_sinfo);
        
        return clients;
}

int SetIdStatus(SINFO *inf, int id, int stat)
{
        pthread_mutex_lock(&inf->m_sinfo);
        inf->ids[id] = stat;
        pthread_mutex_unlock(&inf->m_sinfo);
        
        return 0;
}

int GetIdStatus(SINFO *inf, int id, int *stat)
{
        pthread_mutex_lock(&inf->m_sinfo);
        *stat = inf->ids[id];
        pthread_mutex_unlock(&inf->m_sinfo);
        
        return 0;
}

int AddClient(SINFO *inf, char *cname)
{
        if(GetClientID(inf, cname) > 0)
                return ERNAME;
        int i;
        int stat;
        
        for(i = 1; i < SERVER_T; i++){
                GetIdStatus(inf, i, &stat);
                if(stat == 0){
                        pthread_mutex_lock(&inf->m_sinfo);
                        inf->ids[i] = 1;
                        sprintf(inf->names[i], "%s", cname);
                        pthread_mutex_unlock(&inf->m_sinfo);
                        return 0;
                }
        }
        
        return ERCL;
}

int EraseClient(SINFO *inf, int id)
{
        SetIdStatus(inf, id, 0);
        
        return 0;
}

int SendMSG(SINFO *inf, MSG *msg)
{
        return msgsnd(inf->q_msg, msg, MSG_SIZE, 0);
}

int RecvMSG(SINFO *inf, MSG *msg, int type)
{
        return msgrcv(inf->q_msg, msg, MSG_SIZE, type, 0);
}

int SendBroadcastMSG(SINFO *inf, MSG *msg, int *erase, int e_len)
{
        int i;
        int stat;
        int j;
        
        for(i = 0; i < SERVER_T; i++){
                GetIdStatus(inf, i, &stat);
                if(stat == 0)
                        continue;
                for(j = 0; j < e_len; j++){
                        if(i == erase[j])
                                break;
                }
                if(j < e_len)
                        continue;
                msg->d_type = i;
                SendMSG(inf, msg);
        }
        
        return 0;
}

int SendSMSG(SINFO *inf, SMSG *msg)
{
        return msgsnd(inf->q_serv, msg, SMSG_SIZE, 0);
}

int RecvSMSG(SINFO *inf, SMSG *msg, int type)
{
        return msgrcv(inf->q_serv, msg, SMSG_SIZE, type, 0);
}

int GetClientID(SINFO *inf, char *cname)
{
        int id = -1;
        int i;
        
        pthread_mutex_lock(&inf->m_sinfo);
        for(i = 0; i < SERVER_T; i++){
                if(inf->ids[i] == 0)
                        continue;
                if(strcmp(inf->names[i], cname) == 0){
                        id = i;
                        break;
                }
        }
        
        pthread_mutex_unlock(&inf->m_sinfo);
        
        return id;
}

int GetClientName(SINFO *inf, int id, char *buff)
{
        int rez = -1;
        
        if(id < 1 || id > SERVER_T - 1)
                return -1;
        pthread_mutex_lock(&inf->m_sinfo);
        if(inf->ids[id] > 0){
                printf("%s", inf->names[id]);
                sprintf(buff, "%s", inf->names[id]);
                rez = 0;
        }
        pthread_mutex_unlock(&inf->m_sinfo);
        
        return rez;
}