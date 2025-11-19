#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s number1 number2 ...\n", argv[0]);
        return 1;
    }
    
    double min = atof(argv[1]);
    for (int i = 2; i < argc; i++)
    {
        double tmp = atof(argv[i]);
        if (min > tmp)
            min = tmp;
    }
    
    printf("Min: %f\n", min);
    return 0;
}
