#include "client.h"

int main()
{
        SINFO *inf;
        char data[LONG_STR_SIZE];
        MSG msg;
        
        inf = InitClient
                
        if(inf->q_serv < 0 || inf->q_msg < 0){
                CloseClient(inf);
                return 1;
        }
        if(InitPthread(inf) != 0){
                CloseClient(inf);
                return 1;
        }
        
        while(1){
                scanf("%s", data);
                if(strcmp(data, "close") == 0)
                        break;
                else {
                        msg.d_type = SERVER_T;
                        msg.msg_fl = MNEW;
                        pthread_mutex_lock(&inf->m_id);
                        msg.data_int = inf->id;
                        pthread_mutex_unlock(&inf->m_id);
                        sprintf(msg.data_str_long, "%s", data);
                        SendMSG(inf, &msg);
                }
        }
        
        CloseClient(inf);

        return 0;
}

SINFO *InitClient()
{
        SINFO *inf;
        int i;
        
        inf = malloc(sizeof(SINFO));
        if(inf == NULL){
                perror("Error create SINFO");
                return NULL;
        }
        inf->clients = 0;
        inf->id = -1;
        NewName(inf->name);
        for(i = 0; i < SERVER_T; i++){
                memset(inf->names[i], '\0', STR_SIZE);
                inf->ids[i] = 0;
        }
        inf->q_serv = msgget(SERVER_QKEY, 0);
        if(inf->q_serv < 0)
                perror("Error open server queue");
        inf->q_msg = msgget(MESSAGE_QKEY, 0);
        if(inf->q_msg < 0)
                perror("Error open message queue");
        
        return inf;
}

int InitPthread(SINFO *inf)
{
        pthread_mutex_init(&inf->m_sinfo, NULL);
        if(pthread_create(&inf->p_serv, NULL, TrackingServerQueue, inf) < 0)
                return 1;
        if(pthread_create(&inf->p_msg, NULL, TrackingMessageQueue, inf) < 0)
                return 1;
        
        return 0;
}

void CloseClient(SINFO *inf)
{
        pthread_cancel(inf->p_serv);
        pthread_cancel(inf->p_msg);
        free(inf);
        
}

void *TrackingServerQueue(void *arg)
{
        SMSG smsg;
        SINFO *inf = (SINFO*)arg;
        int id;
        
        smsg.d_type = SERVER_T;
        smsg.msg_fl = RREG;
        sprintf(smsg.data_str, "%s", inf->name);
        SendSMSG(inf, &smsg);
        
        while(1){
                pthread_mutex_lock(&inf->m_id);
                id = inf->id;
                pthread_mutex_unlock(&inf->m_id);
                if(id <= 0){
                        if( RecvSMSG(inf, &smsg, SERVER_T + 1) == -1){
                                perror("Error on receive SMSG");
                                continue;
                        }
                } else {
                        if( RecvSMSG(inf, &smsg, id) == -1){
                                perror("Error on receive SMSG");
                                continue;
                        }
                }
                switch(smsg.msg_fl){
                        case SREG:
                                if(strcmp(smsg.data_str, inf->name) != 0){
                                        SendSMSG(inf, &smsg);
                                        break;
                                }
                                pthread_mutex_lock(&inf->m_id);
                                inf->id = smsg.data_int;
                                pthread_mutex_unlock(&inf->m_id);
                                printf("You registered as %s (%d)\n", inf->name, smsg.data_int);
                        break;
                        case EREG:
                                if(strcmp(smsg.data_str, inf->name) != 0){
                                        SendSMSG(inf, &smsg);
                                        break;
                                }
                                switch(smsg.data_int){
                                        case ERNAME:
                                               printf("EREG: name %s is busy\n", inf->name);
                                        break;
                                        case ERCL:
                                               printf("EREG: chat is full\n");
                                        break;
                                        default:
                                               printf("EREG: unknown error\n");
                                        break;
                                }
                        break;
                        case CREQ:
                                smsg.d_type = SERVER_T;
                                smsg.msg_fl = CREQE;
                                smsg.data_int = inf->id;
                                SendSMSG(inf, &smsg);
                        break;
                        case CAWAY:
                                printf("You disconectedd!\n");
                                pthread_mutex_lock(&inf->m_id);
                                inf->id = -1;
                                pthread_mutex_lock(&inf->m_id);
                                
                                pthread_exit(0);
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
        SINFO *inf_2 = inf;
        char cname[STR_SIZE];
        int id = -1;
        
        while(id <= 0){
                sleep(1);
                pthread_mutex_lock(&inf->m_id);
                id = inf->id;
                pthread_mutex_unlock(&inf->m_id);
        }
        
        while(1){
                printf("%x - %x\n", inf, inf_2);
                pthread_mutex_lock(&inf->m_id);
                id = inf->id;
                pthread_mutex_unlock(&inf->m_id);
                if(id <= 0)
                        pthread_exit(0);
                if( RecvMSG(inf, &msg, id) == -1){
                        perror("Error on receive MSG");
                        continue;
                }
                switch(msg.msg_fl){
                        case MNEWB:
                                if(GetClientName(inf, msg.data_int, cname)){
                                        printf("MNEWB: Error of finding client (%d)\n", msg.data_int);
                                        printf("MNEWB: Client (%d):\n\t%s\n", msg.data_int, msg.data_str_long);
                                } else
                                        printf("MNEWB: %s:\n\t%s\n", cname, msg.data_str_long);
                        break;
                        case CNEW:
                                AddClient(inf, msg.data_str_long, msg.data_int);
                                printf("CNEW: New client %s\n", msg.data_str_long);
                        break;
                        case COUT:
                                EraseClient(inf, msg.data_int);
                                GetClientName(inf, msg.data_int, cname);
                                printf("COUT: Client %s out\n", cname);
                        break;
                        default:
                                printf("Error on receive MSG: Unknown msg_fl (%d)\n", msg.msg_fl);
                        break;
                }
        }
        
        pthread_exit(0);
}

/*int GetClientCount(SINFO *inf)
{
        int clients;
        
        pthread_mutex_lock(&inf->m_sinfo);
        clients = inf->clients;
        pthread_mutex_unlock(&inf->m_sinfo);
        
        return clients;
}
*/
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

int AddClient(SINFO *inf, char *cname, int id)
{
        pthread_mutex_lock(&inf->m_sinfo);
        inf->ids[id] = 1;
        sprintf(inf->names[id], "%s", cname);
        inf->clients++;
        pthread_mutex_unlock(&inf->m_sinfo);
        
        return 0;
}

int EraseClient(SINFO *inf, int id)
{
        SetIdStatus(inf, id, 0);
        
        pthread_mutex_lock(&inf->m_sinfo);
        inf->clients--;
        pthread_mutex_unlock(&inf->m_sinfo);
        
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
        
        pthread_mutex_lock(&inf->m_sinfo);
        if(inf->ids[id] > 0){
                sprintf(buff, "%s", inf->names[id]);
                rez = 0;
        }
        pthread_mutex_unlock(&inf->m_sinfo);
        
        return rez;
}

int NewName(char *buff)
{
        printf("Get your name:\n");
        scanf("%s", buff);
        
        return 0;
}
