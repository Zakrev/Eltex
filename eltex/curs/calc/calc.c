#include <stdio.h>
#include "calc.h"

#define ADD_ITEM 1
#define SUB_ITEM 2
#define MUL_ITEM 3
#define DIV_ITEM 4
#define CADD_ITEM 5
#define CSUB_ITEM 6
#define CMUL_ITEM 7
#define CDIV_ITEM 8

#define EXIT_ITEM 9

int exe_menu()
{
        int menu_item = -1;

        printf("\n\tWelcome to Calculator!\n"
        "Float operations:\n"
        "\t1) Add: a + b\n"
        "\t2) Subtraction: a - b\n"
        "\t3) Multiplication: a * b\n"
        "\t4) Division: a / b\n"
        "Complex operations:\n"
        "\t5) Add: (a + bi) + (c + di)\n"
        "\t6) Substraction: (a + bi) - (c + di)\n"
        "\t7) Multiplication: (a + bi) * (c + di)\n"
        "\t8) Division: (a + bi) / (c + di)\n"
        "\n"
        "\t%d) Exit\n", EXIT_ITEM);
        printf("$");
        while(menu_item < 1 || menu_item > 8){
                scanf("%d", &menu_item);
                if(menu_item == EXIT_ITEM)
                        return EXIT_ITEM;
        }

        return menu_item;
}

void get_input_var(double* a, double* b)
{
        printf("a:");
        scanf("%lf", a);
        printf("b:");
        scanf("%lf", b);
}

void get_input_complex_var(double* a, double* b, double* c, double* d)
{
        get_input_var(a, b);
        printf("c:");
        scanf("%lf", c);
        printf("d:");
        scanf("%lf", d);
}

void exe_menu_item(int menu_item)
{
        double a, b, c, d;
        COMPLEX com_a, com_b, com_c;

        switch(menu_item){
                case ADD_ITEM:
                        get_input_var(&a, &b);
                        printf("%.2f + %.2f = %.2f\n", a, b,  mAdd(a, b));
                break;
                case SUB_ITEM:
                        get_input_var(&a, &b);
                        printf("%.2f - %.2f = %.2f\n", a, b,  mSubtraction(a, b));
                break;
                case MUL_ITEM:
                        get_input_var(&a, &b);
                        printf("%.2f * %.2f = %.2f\n", a, b,  mMultiplication(a, b));
                break;
                case DIV_ITEM:
                        get_input_var(&a, &b);
                        printf("%.2f / %.2f = %.2f\n", a, b,  mDivision(a, b));
                break;
                case CADD_ITEM:
                        get_input_complex_var(&a, &b, &c, &d);
                        com_a.re = a;
                        com_a.im = b;
                        com_b.re = c;
                        com_b.im = d;
                        com_c = mAddComplex(com_a, com_b);
                        printf("(%.2f + %.2fi) + (%.2f + %.2fi) = %.2f + %.2fi\n", a, b, c, d, com_c.re, com_c.im);
                break;
                case CSUB_ITEM:
                        get_input_complex_var(&a, &b, &c, &d);
                        com_a.re = a;
                        com_a.im = b;
                        com_b.re = c;
                        com_b.im = d;
                        com_c = mSubComplex(com_a, com_b);
                        printf("(%.2f + %.2fi) - (%.2f + %.2fi) = %.2f + %.2fi\n", a, b, c, d, com_c.re, com_c.im);
                break;
                case CMUL_ITEM:
                        get_input_complex_var(&a, &b, &c, &d);
                        com_a.re = a;
                        com_a.im = b;
                        com_b.re = c;
                        com_b.im = d;
                        com_c = mMulComplex(com_a, com_b);
                        printf("(%.2f + %.2fi) * (%.2f + %.2fi) = %.2f + %.2fi\n", a, b, c, d, com_c.re, com_c.im);
                break;
                case CDIV_ITEM:
                        get_input_complex_var(&a, &b, &c, &d);
                        com_a.re = a;
                        com_a.im = b;
                        com_b.re = c;
                        com_b.im = d;
                        com_c = mDivComplex(com_a, com_b);
                        printf("(%.2f + %.2fi) / (%.2f + %.2fi) = %.2f + %.2fi\n", a, b, c, d, com_c.re, com_c.im);
                break;
        }
}

int main()
{
        int menu_item = -1;

        while(menu_item != EXIT_ITEM){
                menu_item = exe_menu();
                exe_menu_item(menu_item);
        }

        return 0;
}