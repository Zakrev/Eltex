#include "zd2.h"

int main()
{
        int qd;
        MMSG data;
        
        qd = msgget(QKEY, 0777);
        if(qd < 0){
                perror("Error open queue ");
                return 1;
        }
        /*Читаем*/
        if(msgrcv(qd, &data, MMSG_SIZE, SERVER_T, 0) < 0)
                perror("Error on receive message");
        else
                printf("%s: %s\n", data.from, data.msg);
        /*Пишем*/
        sprintf(data.from, "Client");
        sprintf(data.msg, "Hi server");
        data.type = CLIENT_T;
        if(msgsnd(qd, &data, MMSG_SIZE, 0) < 0)
                perror("Error send messgae");
        
        return 0;
}