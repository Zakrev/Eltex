#include "s_inet.h"

int main()
{
        SINF *sinf;
        char c;
        
        sinf = InitSINF();
        if(sinf == NULL)
                exit(1);
        printf("Ceate PTH 0: ");
        if(CreatePTH(sinf) < 0)
                goto eexit1;
        printf("OK\n");
        printf("StartServices: ");
        if(StartServices(sinf) < 0)
                goto eexit1;
        printf("OK\n");

        scanf("%c", &c);

        printf("Shutdown: ");        
        RemoveSINF(sinf);
        printf("OK\n");

        return 0;

        eexit1: 
                RemoveSINF(sinf);
                return -1;
}