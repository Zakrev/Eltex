#include <stdio.h>
#include <stdlib.h>

struct t {
        char a;
        int b;
} __attribute__((packed));

int main()
{
        char test[5];
        char* a;
        int* b;
        struct t* ptr;

        a = test;
        b = (int*)(a + 1);

        *a = 'A';
        *b = 13;

        printf("(a,b): %c -  %d\n", *a, *b);

        ptr = (struct t*)test;

        printf("(ptr): %c - %d\n", ptr->a, ptr->b);
        printf("Sizeof: %d\n", sizeof(struct t));

        return 0;
}