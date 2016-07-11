#include <stdio.h>
#include <dlfcn.h>

int exe_menu()
{
        return 0;
}


void exe_menu_item(int menu_item)
{

}

double (*InMAdd) (double, double);

int main()
{
        /*int menu_item = -1;

        while(menu_item != EXIT_ITEM){
                menu_item = exe_menu();
                exe_menu_item(menu_item);
        }*/
        void* handle;

        handle = dlopen("./add.so", RTLD_NOW);
        
        InMAdd = dlsym(handle, "mAdd");

        printf("%f\n", InMAdd(20.0, 30.0));

        return 0;
}