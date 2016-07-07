#include <stdio.h>
#include <stdlib.h>

#define COMPLEX struct mcomplex

COMPLEX {
        int re;
        int im;
};

COMPLEX SumComplex(COMPLEX a, COMPLEX b)
{
        COMPLEX c;

        c.re = a.re + b.re;
        c.im = a.im + b.im;

        return c;
}

COMPLEX DifferenceComplex(COMPLEX a, COMPLEX b)
{
        b.re *= -1;
        b.im *= -1;

        return SumComplex(a, b);
}

int main()
{
        COMPLEX a;
        COMPLEX b;
        COMPLEX c;

        a.re = 2;
        a.im = 5;
        b.re = 7;
        b.im = -3;

        c = DifferenceComplex(a, b);

        printf("c: Re(%d) Im(%di)\n", c.re, c.im);

        return 0;
}