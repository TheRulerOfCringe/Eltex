#include <math.h>

double fdivide(double x, double y)
{
    if (y == 0)
        return NAN;
    return 1 + (x / y);
}