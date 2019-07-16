#include "MathFunctions.h"

double power(double base,int exponent)
{
    double result = base;

    if(exponent == 0)
        return 1;

    for(int i = 1;i < exponent;++i)
    {
        result *= base;
    }
    return result;
}
