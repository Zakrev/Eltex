#include "complex.h"

struct mComplex mAddComplex(struct mComplex a, struct mComplex b)
{
        struct mComplex c;

        c.re = a.re + b.re;
        c.im = a.im + b.im;

        return c;
}

struct mComplex mSubComplex(struct mComplex  a, struct mComplex b)
{
        b.re *= -1;
        b.im *= -1;

        return mAddComplex(a, b);
}