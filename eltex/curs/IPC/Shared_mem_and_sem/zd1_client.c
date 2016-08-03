#include "zd1.h"

int main()
{
        int sem_d;
        int mem_d;
        int bits;
        MMSG data;
        
        mem_d = shm_open(MFNAME, O_RDWR, 0);
        if(mem_d <= 0){
                perror("Error on open"MFNAME);
                return 1;
        }
        sem_d = semget(SEMKEY, 0, 0);
        if(sem_d <= 0){
                perror("Error on ger sem");
                return 1;
        }
        /*Читаем*/
        while(1){
                if(semop(sem_d, zd1_lock, 2) < 0)
                        perror("Error on lock sem");
                else {
                        lseek(mem_d, SERVER_POS, SEEK_SET);
                        bits = read(mem_d, &data, MMSG_SIZE);
                        if(semop(sem_d, &zd1_unlock, 1) < 0){
                                perror("Error on unlock sem");
                                break;
                        }
                        if(bits > 0)
                                break;
                }
        }
        printf("%s: %s\n", data.from, data.msg);
        /*Пишем*/
        sprintf(data.from, "Client");
        sprintf(data.msg, "Ну hello...");
        if(semop(sem_d, zd1_lock, 2) < 0)
                perror("Error on lock sem");
        else {
                lseek(mem_d, CLIENT_POS, SEEK_SET);
                bits = write(mem_d, &data, MMSG_SIZE);
                if(semop(sem_d, &zd1_unlock, 1) < 0)
                        perror("Error on unlock sem");
        }
        printf("write %d byte\n", bits);

        return 0;
}