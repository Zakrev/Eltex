#include "zd2.h"

int main()
{
        int qd;
        MMSG data;
        
        qd = msgget(QKEY, IPC_CREAT | 0777);
        if( qd < 0 ){
                perror("Error on create queue ");
                return 1;
        }
        /*Ставим в очередь*/
        sprintf(data.from, "Server");
        sprintf(data.msg, "Hello client!");
        data.type = SERVER_T;
        if(msgsnd(qd, &data, MMSG_SIZE, 0) < 0)
                perror("Error send messgae");
        /*Забираем из очереди*/
        if(msgrcv(qd, &data, MMSG_SIZE, CLIENT_T, 0) < 0)
                perror("Error on receive message");
        else
                printf("%s: %s\n", data.from, data.msg);
        msgctl(qd, IPC_RMID, NULL);

        return 0;
}
