#include "zd1.h"

int main()
{
        int mem_d;
        void *mem_map;
        int bits;
        int sem_d;
        MMSG data;
        
        mem_d = shm_open(MFNAME, O_CREAT | O_RDWR, 0777);
        if(mem_d <= 0){
                perror("Error open "MFNAME);
                return 1;
        }
        mem_map = mmap(NULL, MEMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_d, 0);
        if(mem_map == NULL){
                perror("Eror mmap");
                return 1;
        }
        sem_d = semget(SEMKEY, 1, IPC_CREAT | 0777);
        if(sem_d < 0){
                perror("Error create sem");
                munmap(mem_map, MEMSIZE);
                shm_unlink(MFNAME);
                return 1;
        }
        /*Создаем сообщение*/
        sprintf(data.msg, "Hello!");
        sprintf(data.from, "Server");
        /*Записываем*/
        if(semop(sem_d, zd1_lock, 2) < 0)
                perror("Error on sem lock");
        else {
                lseek(mem_d, SERVER_POS, SEEK_SET);
                bits = write(mem_d, &data, MMSG_SIZE);
                printf("write %d byte\n", bits);
                if(semop(sem_d, &zd1_unlock, 1) < 0)
                        perror("Error on sem unlock");
        }
        /*Читаем*/
        while(1){
                if(semop(sem_d, zd1_lock, 2) < 0)
                        perror("Error on sem lock");
                else {
                        lseek(mem_d, CLIENT_POS, SEEK_SET);
                        bits = read(mem_d, &data, MMSG_SIZE);
                        if(semop(sem_d, &zd1_unlock, 1) < 0)
                                perror("Error on sem unlock");
                        if(bits > 0){
                                printf("%s: %s\n", data.from, data.msg);
                                break;
                        }
                }
        }
        munmap(mem_map, MEMSIZE);
        shm_unlink(MFNAME);
        semctl(sem_d, 0, IPC_RMID);
        
        return 0;
}