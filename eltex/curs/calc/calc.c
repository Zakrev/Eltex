#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

struct Menu {
        void** functions;
        char** func_desc;
        int func_count;
};

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
        char func_name[128];
        char func_desc[128];
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
        printf("\tlib %s include %d functions.\n", lib_name, *function_count);
        for(i = 0; i < *function_count && lenght != -1; i++){
                lenght +=  ParseInfo(function_info + lenght, func_name, func_desc);
                strcpy(menu->func_desc[menu->func_count], func_desc);
                menu->functions[menu->func_count] = dlsym(lib_id, func_name);
                menu->func_count += 1;
        }
}

int CreateMenu(struct Menu* menu)
{
        char lib_name[20] = "";
        int i;
        
        menu->functions = malloc(sizeof(void*) * 10);
        menu->func_desc = malloc(sizeof(char*) * 10);
        for(i = 0; i < 10; i++)
                menu->func_desc[i] = malloc(129);
        menu->func_count = 0;
        
        printf("Load library:\n");
        scanf("%s", lib_name);
        while(strcmp(lib_name, "end") != 0){
                LoadAndAddLib(lib_name, menu);
                scanf("%s", lib_name);
        }

        return 0;
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

        while(menu_item < 1 || menu.func_count + 1 < menu_item){
                printf("$ ");
                scanf("%d", &menu_item);
        }

        return menu_item;
}


void GetInputVar(double* a, double* b)
{
        printf("a: ");
        scanf("%lf", a);
        printf("b: ");
        scanf("%lf", b);
}

void ExeMenuItem(struct Menu menu, int menu_item)
{
        if(menu_item > menu.func_count)
                return;

        double (*func_init) (double, double);
        double a, b;

        GetInputVar(&a, &b);
        func_init = menu.functions[menu_item - 1];
        printf("%s = %.2f\n", menu.func_desc[menu_item - 1], func_init(a, b));
}

int main()
{
        int menu_item = -1;
        struct Menu menu;

        CreateMenu(&menu);
        while(menu_item != menu.func_count + 1){
                menu_item = ExeMenu(menu);
                ExeMenuItem(menu, menu_item);
        }
        
        return 0;
}