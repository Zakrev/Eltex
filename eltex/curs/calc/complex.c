#include "complex.h"

int function_count = 4;
char function_info[300] = "mAddComplex.Add complex: (a + bi) + (c + di).mSubComplex.Substraction complex: (a + bi) - (c + di).mMulComplex.Multiplication complex: (a + bi) * (c + di).mDivComplex.Division complex: (a + bi) / (c + di)";

struct mComplex mAddComplex(struct mComplex a, struct mComplex b)
{
        struct mComplex c;

        c.re = a.re + b.re;
        c.im = a.im + b.im;

        return c;
}

struct mComplex mSubComplex(struct mComplex a, struct mComplex b)
{
        b.re *= -1;
        b.im *= -1;

        return mAddComplex(a, b);
}

struct mComplex mMulComplex(struct mComplex a, struct mComplex b)
{
        struct mComplex c;

        c.re = a.re * b.re - a.im * b.im;
        c.im = a.re * b.im + a.im * b.re;

        return c;
}

struct mComplex mDivComplex(struct mComplex a, struct mComplex b)
{
        struct mComplex c;

        c.re = (a.re * b.re + a.im * b.im) / (b.re * b.re + b.im * b.im);
        c.im = (a.re * b.im - a.im * b.re) / (b.re * b.re + b.im * b.im);

        return c;
}