#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

#pragma comment(lib, "libs/add.dll")
#pragma comment(lib, "libs/subtract.dll") 
#pragma comment(lib, "libs/multiply.dll")
#pragma comment(lib, "libs/fadd.dll")
#pragma comment(lib, "libs/fsubtract.dll")
#pragma comment(lib, "libs/fmultiply.dll")
#pragma comment(lib, "libs/fdivide.dll")

__declspec(dllimport) int add(int a, int b);
__declspec(dllimport) int subtract(int a, int b);
__declspec(dllimport) int multiply(int a, int b);
__declspec(dllimport) double fadd(double a, double b);
__declspec(dllimport) double fsubtract(double a, double b);
__declspec(dllimport) double fmultiply(double a, double b);
__declspec(dllimport) double fdivide(double a, double b);

// Объявляем правильные типы указателей на функции
typedef int (*int_func_t)(int, int);
typedef double (*double_func_t)(double, double);

// Глобальные указатели на функции
int_func_t add = NULL;
int_func_t subtract = NULL;
int_func_t multiply = NULL;

double_func_t fadd = NULL;
double_func_t fsubtract = NULL;
double_func_t fmultiply = NULL;
double_func_t fdivide = NULL;

// Глобальные handles для библиотек
HMODULE handles[20];
int handle_count = 0;

int_func_t select_operation(int choice)
{
    switch (choice)
    {
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
        if (sscanf(buffer, "%d %c", &a, &extra) == 1)
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
        if (sscanf(buffer, "%d %c", &b, &extra) == 1)
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
            if (sscanf(buffer, "%d %c", &choice, &extra) == 1)
            {
                if (choice < 1 || choice > 3) {
                    printf("Пожалуйста, введите число от 1 до 3!\n");
                    continue;
                }
                
                int_func_t operation = select_operation(choice);
                if (operation) {
                    return operation(a, b);
                } else {
                    printf("Функция не загружена!\n");
                }
            }
            else
                printf("Пожалуйста, введите ЦЕЛОЕ число от 1 до 3 без лишних символов!\n");
        }
    }
    return 0;
}

double_func_t fselect_operation(int choice)
{
    switch (choice)
    {
    case 2: return fsubtract;
    case 3: return fmultiply;
    case 4: return fdivide;
    default: return fadd;
    }
}

double float_calc()
{
    double a, b;
    bool flag = true;
    char buffer[100];
    char extra;

    while (flag)
    {
        printf("Введите первое вещественное число: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (sscanf(buffer, "%lf %c", &a, &extra) == 1)
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
        if (sscanf(buffer, "%lf %c", &b, &extra) == 1)
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
            if (sscanf(buffer, "%d %c", &choice, &extra) == 1)
            {
                if (choice < 1 || choice > 4) {
                    printf("Пожалуйста, введите число от 1 до 4!\n");
                    continue;
                }
                
                double_func_t operation = fselect_operation(choice);
                if (operation) {
                    return operation(a, b);
                } else {
                    printf("Функция не загружена!\n");
                }
            }
            else
                printf("Пожалуйста, введите ЦЕЛОЕ число от 1 до 4 без лишних символов!\n");
        }
    }
    return 0.0;
}

int nsum(int n, ...)
{
    int result = 0;
    va_list factor;
    va_start(factor, n);
    for (int i = 0; i < n; i++) {
        result += va_arg(factor, int);
    }
    va_end(factor);
    return result;
}

int nsum_calc()
{
    bool flag = true;
    char buffer[100];
    int n = 0, m[5] = { 0 };

    while (flag)
    {
        printf("Введите количество переменных для суммирования (2 <= n <= 5) : ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            char extra;
            if (sscanf(buffer, "%d %c", &n, &extra) == 1)
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
            printf("Введите переменную номер %d: ", i + 1);

            if (fgets(buffer, sizeof(buffer), stdin) != NULL)
            {
                char extra;
                if (sscanf(buffer, "%d %c", &m[i], &extra) == 1)
                    flag = false;
                else
                    printf("Пожалуйста, введите ЦЕЛОЕ число без лишних символов!\n");
            }
        }
    }

    return nsum(n, m[0], m[1], m[2], m[3], m[4]);
}

void load_math_functions() {
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("./libs/*.dll", &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No DLL files found in ./libs directory!\n");
        return;
    }

    do {
        char path[100];
        sprintf(path, "./libs/%s", findData.cFileName);

        HMODULE handle = LoadLibrary(path);
        if (handle) {
            char func_name[50];
            strcpy(func_name, findData.cFileName);
            func_name[strlen(func_name) - 4] = '\0';

            FARPROC func = GetProcAddress(handle, func_name);
            if (func) {
                printf("Found: %s\n", func_name);
                
                // Сохраняем handle
                handles[handle_count++] = handle;
                
                // Присваиваем указатели на функции с правильным приведением типов
                if (strcmp(func_name, "add") == 0) add = (int_func_t)func;
                else if (strcmp(func_name, "subtract") == 0) subtract = (int_func_t)func;
                else if (strcmp(func_name, "multiply") == 0) multiply = (int_func_t)func;
                else if (strcmp(func_name, "fadd") == 0) fadd = (double_func_t)func;
                else if (strcmp(func_name, "fsubtract") == 0) fsubtract = (double_func_t)func;
                else if (strcmp(func_name, "fmultiply") == 0) fmultiply = (double_func_t)func;
                else if (strcmp(func_name, "fdivide") == 0) fdivide = (double_func_t)func;
            } else {
                FreeLibrary(handle);
            }
        }
    } while (FindNextFile(hFind, &findData));
    FindClose(hFind);
}

void free_math_functions() {
    for (int i = 0; i < handle_count; i++) {
        FreeLibrary(handles[i]);
    }
}

int main()
{
    setlocale(LC_ALL, "RUS");

    // Загружаем функции из DLL
    //load_math_functions();

    printf("Добро пожаловать в меню программы калькулятора.\n\n");
    int choice = 0, result = 0;
    char buffer[100];
    
    while (choice != -1)
    {
        printf("Вам доступны следующие опции для ввода:\n");
        printf("1 - целочисленный калькулятор;\n");
        printf("2 - калькулятор чисел с плавающей точкой;\n");
        printf("3 - просуммировать n целых чисел;\n");
        printf("-1 - выход из программы;\n");
        printf("Ввод: ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            char extra;
            if (sscanf(buffer, "%d %c", &choice, &extra) == 1)
            {
                switch (choice)
                {
                case 1:
                    printf("Вы запустили целочисленный калькулятор (на одну операцию)\n");
                    result = int_calc();
                    printf("Итог вычислений следующий: %d\n\n", result);
                    break;
                case 2:
                    printf("Вы запустили калькулятор чисел с плавающей точкой (на одну операцию)\n");
                    double fresult = float_calc();
                    printf("Итог вычислений следующий: %f\n\n", fresult);
                    break;
                case 3:
                    printf("Вы решили просуммировать n целых чисел\n");
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

    // Освобождаем библиотеки
    free_math_functions();

    return 0;
}