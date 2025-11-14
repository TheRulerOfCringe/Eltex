#include <stdio.h>
#include <locale.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h> // только ради NAN подключил

// Начало кода целочисленного калькулятора

int add(int x, int y)
{
    return x + y;
}

int subtract(int x, int y)
{
    return x - y;
}

int multiply(int x, int y)
{
    return x * y;
}

int (*select(int choice)) (int, int)
{
    switch (choice)
    {
    // case 1 нет потому что в дефолт ушла :)
    case 2: return subtract;
    case 3: return multiply;
    default: return add;
    }
}

int int_calc()
{
    int a, b;
    bool flag = true;
    char buffer[100];
    char extra;

    while (flag)
    {
        printf("Введите первое целое число: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (sscanf_s(buffer, "%d %c", &a, &extra, 1) == 1)
            flag = false;
        else
            printf("Пожалуйста, введите ЦЕЛОЕ число без лишних символов!\n");
    }
    printf("Вы ввели a = %d\n", a);
    flag = true;

    while (flag)
    {
        printf("Введите второе целое число: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (sscanf_s(buffer, "%d %c", &b, &extra, 1) == 1)
            flag = false;
        else
            printf("Пожалуйста, введите ЦЕЛОЕ число без лишних символов!\n");
    }

    printf("Вы ввели b = %d\n", b);
    flag = true;

    printf("Вам доступны следующие опции для ввода:\n");
    printf("1 - сумма;\n");
    printf("2 - разность;\n");
    printf("3 - умножение;\n");

    int choice = 0;
    while (flag)
    {
        printf("Введите номер операции: ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            char extra;
            if (sscanf_s(buffer, "%d %c", &choice, &extra, 1) == 1)
            {
                int (*operation)(int, int) = select(choice);
                return operation(a, b);
            }
            else
                printf("Пожалуйста, введите ЦЕЛОЕ число от 1 до 3 без лишних символов!\n");
        }
    }
}

// конец кода целочисленного калькулятора

// начало кода калькулятора чисел с плавающей точкой


float fadd(float x, float y)
{
    return x + y;
}

float fsubtract(float x, float y)
{
    return x - y;
}

float fmultiply(float x, float y)
{
    return x * y;
}

float fdivide(float x, float y)
{
    if (y == 0)
        return NAN;
    return x / y;
}

float (*fselect(int choice)) (float, float)
{
    switch (choice)
    {
        // case 1 нет потому что в дефолт ушла :)
    case 2: return fsubtract;
    case 3: return fmultiply;
    case 4: return fdivide;
    default: return fadd;
    }
}

float float_calc()
{
    float a, b;
    bool flag = true;
    char buffer[100];
    char extra;

    while (flag)
    {
        printf("Введите первое вещественное число: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (sscanf_s(buffer, "%f %c", &a, &extra, 1) == 1)
            flag = false;
        else
            printf("Пожалуйста, введите одно лишь число без лишних символов!\n");
    }
    printf("Вы ввели a = %g\n", a);
    flag = true;

    while (flag)
    {
        printf("Введите второе вещественное число: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (sscanf_s(buffer, "%f %c", &b, &extra, 1) == 1)
            flag = false;
        else
            printf("Пожалуйста, введите одно лишь число без лишних символов!\n");
    }

    printf("Вы ввели b = %g\n", b);
    flag = true;

    printf("Вам доступны следующие опции для ввода:\n");
    printf("1 - сумма;\n");
    printf("2 - разность;\n");
    printf("3 - умножение;\n");
    printf("4 - деление;\n");

    int choice = 0;
    while (flag)
    {
        printf("Введите номер операции: ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            char extra;
            if (sscanf_s(buffer, "%d %c", &choice, &extra, 1) == 1)
            {
                float (*operation)(float, float) = fselect(choice);
                return operation(a, b);
            }
            else
                printf("Пожалуйста, введите ЦЕЛОЕ число от 1 до 4 без лишних символов!\n");
        }
    }
}

// конец кода калькулятора чисел с плавающей точкой

// начало кода функции с нефиксированным числом параметров

int nsum(int n, ...)
{
    int result = 0;
    va_list factor; //указатель va_list
    va_start(factor, n); // устанавливаем указатель
    for (int i = 0; i < n; i++) {
        result += va_arg(factor, int); // получаем значение текущего параметра типа int
    }
    va_end(factor); // завершаем обработку параметров
    return result;
}

int nsum_calc()
{
    bool flag = true;
    char buffer[100];
    int n = 0, m[5] = {0};

    while (flag)
    {
        printf("Введите количество переменных для суммирования (2 <= n <= 5) : ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            char extra;
            if (sscanf_s(buffer, "%d %c", &n, &extra, 1) == 1)
            {
                if (n < 2 || n > 5)
                    printf("Пожалуйста, введите целое число больше 1 и меньше 6!\n");
                else
                    flag = false;
            }
            else
                printf("Пожалуйста, введите ЦЕЛОЕ число без лишних символов!\n");
        }
    }

    for (int i = 0; i < n; i++)
    {
        flag = true;
        while (flag)
        {
            printf("Введите переменную номер %d: ", i+1);

            if (fgets(buffer, sizeof(buffer), stdin) != NULL)
            {
                char extra;
                if (sscanf_s(buffer, "%d %c", &m[i], &extra, 1) == 1)
                    flag = false;
                else
                    printf("Пожалуйста, введите ЦЕЛОЕ число без лишних символов!\n");
            }
        }
    }

    return nsum(n, m[0], m[1], m[2], m[3], m[4]);
}

// конец кода функции с нефиксированным числом параметров

int main()
{
	setlocale(LC_ALL, "RUS");
    
	printf("Добро пожаловать в меню программы калькулятора.\n\n");
    int choice = 0, result = 0;
	char buffer[100];
    while (choice != -1)
    {
        printf("Вам доступны следующие опции для ввода:\n");
        printf("1 - целочисленный калькулятор;\n");
        printf("2 - калькулятор чисел с плавающей точкой;\n");
        printf("3 - просуммировать n челых чисел;\n");
        printf("-1 - выход из проргаммы;\n");
        printf("Ввод: ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            char extra;
            if (sscanf_s(buffer, "%d %c", &choice, &extra, 1) == 1)
            {
                switch (choice)
                {
                case 1:
                    printf_s("Вы запустили целочисленный калькулятор (на одну операцию)\n");
                    result = int_calc();
                    printf("Итог вычислений следующий: %d\n\n", result);
                    break;
                case 2:
                    printf_s("Вы запустили калькулятор чисел с плавающей точкой (на одну операцию)\n");
                    float fresult = float_calc();
                    printf("Итог вычислений следующий: %f\n\n", fresult);
                    break;
                case 3:
                    printf_s("Вы решили просуммировать n целых чисел\n");
                    result = nsum_calc();
                    printf("Итог суммирования следующий: %d\n\n", result);
                    break;
                case -1:
                    printf("Пока-пока!\n");
                    break;
                default:
                    printf("Такого я не понял, давай по новой.\n");
                }
            }
            else
            {
                printf("Пожалуйста, введите ЦЕЛОЕ число без лишних символов!\n");
            }
        }
    }
	return 0;
}