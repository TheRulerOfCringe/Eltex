#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s number1 number2 ...\n", argv[0]);
        return 1;
    }
    
    double sum = 0;
    for (int i = 1; i < argc; i++)
        sum += atof(argv[i]);
    
    printf("Sum: %f\n", sum);
    return 0;
}
