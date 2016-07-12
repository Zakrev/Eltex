#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#define MAX_FUNC_COUNT 10
#define MAX_STR_LEN 128
#define ER_CANT_EXE_POPEN "Error: Cant execute command popen\n"

struct Menu {
        char lib_name[MAX_FUNC_COUNT][MAX_STR_LEN];
        char func_name[MAX_FUNC_COUNT][MAX_STR_LEN];
        char func_desc[MAX_FUNC_COUNT][MAX_STR_LEN];
        int func_count;
};

int scan_int(int* value)
{
        char buff[32];

        scanf("%s", buff);
        if(buff[0] < '0' || buff[0] > '9')
                return -1;
        *value = atoi(buff);

        return 0;
}

int scan_double(double* value)
{
        char buff[64];

        scanf("%s", buff);
        if(buff[0] < '0' || buff[0] > '9')
                return -1;
        *value = atof(buff);

        return 0;
}

size_t FindCh(char* str, char ch)
{
        size_t i;

        for(i = 0; str[i] != '\0' && str[i] != ch; i++);

        return i;
}

size_t ParseInfo(char* info, char* func_name, char* func_descr)
{
        size_t all_lenght = 0;
        size_t lenght = FindCh(info, '.');
        if(lenght == 0)
                return -1;
        all_lenght = lenght + 1;
        strncpy(func_name, info, lenght);
        func_name[lenght] = '\0';
        info = info + lenght + 1;
        lenght = FindCh(info, '.');
        all_lenght += lenght + 1;
        strncpy(func_descr, info, lenght);
        func_descr[lenght] = '\0';

        return all_lenght;
}

void LoadAndAddLib(char* lib_name, struct Menu* menu)
{
        void* lib_id;
        char* error_open;
        int* function_count;
        char* function_info;
        char func_name[MAX_STR_LEN];
        char func_desc[MAX_STR_LEN];
        int i;
        size_t lenght = 0;

        lib_id = dlopen(lib_name, RTLD_NOW);
        error_open = dlerror();
        if(error_open != NULL){
                printf("Error: %s\n", error_open);
                return;
        }
        function_count = (int*)dlsym(lib_id, "function_count");
        function_info = (char*)dlsym(lib_id, "function_info");
        for(i = 0; i < *function_count && lenght != -1 && menu->func_count < MAX_FUNC_COUNT; i++){
                lenght +=  ParseInfo(function_info + lenght, func_name, func_desc);
                strcpy(menu->func_desc[menu->func_count], func_desc);
                strcpy(menu->func_name[menu->func_count], func_name);
                strcpy(menu->lib_name[menu->func_count], lib_name);
                menu->func_count += 1;
        }
        dlclose(lib_id);
        printf("\tlib %s include %d functions.\n", lib_name, i);
}

int CreateMenu(struct Menu* menu)
{
        char lib_name[254] = "./";
        FILE* popen_res;
       
        system("clear");

        popen_res = (FILE*)popen("ls plugin/*.so 2> \\dev\\null", "r");
        if(!popen_res){
                printf(ER_CANT_EXE_POPEN);
                return -1;
        }

        menu->func_count = 0;
        
        printf("Load library.\n");
        while(fgets(lib_name+2, sizeof(lib_name) - 2, popen_res) != NULL){
                lib_name[FindCh(lib_name, '\n')] = '\0';
                LoadAndAddLib(lib_name, menu);
        }
        pclose(popen_res);
        printf("\tLoaded %d functions.\n", menu->func_count);
        return 0;
}

void ClearMenu(struct Menu* menu)
{
}

int ExeMenu(struct Menu menu)
{
        int i;
        int menu_item = -1;

        printf("Welcome to Calculator!\n");
        for(i = 0; i < menu.func_count; i++){
                printf("\t%d) %s\n", i + 1, menu.func_desc[i]);
        }
        printf("\t%d) Exit.\n", i + 1);

        while(menu_item < 1 || menu.func_count + 1 < menu_item || i == -1){
                printf("$ ");
                i = scan_int(&menu_item);
        }

        return menu_item;
}


void GetInputVar(double* a, double* b)
{
        int i = -1;

        while(i == -1){
                printf("a: ");
                i = scan_double(a);
        }
        i = -1;
        while(i == -1){
                printf("b: ");
                i = scan_double(b);
        }
}

void ExeMenuItem(struct Menu menu, int menu_item)
{
        if(menu_item > menu.func_count)
                return;

        double (*func_init) (double, double);
        double a, b;
        char* error_open;
        void* lib_id;

        lib_id = dlopen(menu.lib_name[menu_item - 1], RTLD_NOW);
        error_open = dlerror();
        if(error_open != NULL){
                printf("Error: %s\n", error_open);
                return;
        }

        system("clear");
        printf("%s\n", menu.func_desc[menu_item - 1]);
        GetInputVar(&a, &b);
        
        func_init = (double*)dlsym(lib_id, menu.func_name[menu_item - 1]);
        system("clear");
        printf("%s = %.2f\n", menu.func_desc[menu_item - 1], func_init(a, b));
        dlclose(lib_id);
}

int main()
{
        int menu_item = -1;
        struct Menu menu;

        if( CreateMenu(&menu) != 0 )
                return 0;
        while(menu_item != menu.func_count + 1){
                menu_item = ExeMenu(menu);
                ExeMenuItem(menu, menu_item);
        }
        ClearMenu(&menu);
        system("clear");

        return 0;
}