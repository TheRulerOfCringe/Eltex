#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>     // для закомменченного кода от Microsoft
#include <stdlib.h>   // для EXIT_FAILURE
#include <locale.h>

#define S_IRUSR 0400
#define S_IWUSR 0200 
#define S_IXUSR 0100
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001

void printPravaBukvami(unsigned int rights)
{
    unsigned int entry_mask = 511; // 0000 0000 0000 0000 0000 0001 1111 1111 - взять первых 9 битов
    rights = rights & entry_mask;

    printf("\nRights for this file is: ");
    printf((rights & S_IRUSR) ? "r" : "-");
    printf((rights & S_IWUSR) ? "w" : "-");
    printf((rights & S_IXUSR) ? "x" : "-");
    printf((rights & S_IRGRP) ? "r" : "-");
    printf((rights & S_IWGRP) ? "w" : "-");
    printf((rights & S_IXGRP) ? "x" : "-");
    printf((rights & S_IROTH) ? "r" : "-");
    printf((rights & S_IWOTH) ? "w" : "-");
    printf((rights & S_IXOTH) ? "x" : "-");
    printf("\n");
}

void printPrava8richnoe(unsigned int rights)
{
    unsigned int entry_mask = 511; // 0000 0000 0000 0000 0000 0001 1111 1111 - взять первых 9 битов
    rights = rights & entry_mask;

    printf("\nRights for this file is: %o\n", rights);
}

void printPrava2ichnoe(unsigned int rights)
{
    printf("\nRights for this file is: ");
    for (int i = 8; i >= 0; i--)
        printf("%d", (rights >> i) & 1);
    printf("\n");
}

unsigned int chmod_verbose (unsigned int st_mode, char* str)
{

    for (int i = 31; i >= 0; i--)
        printf("%d", (st_mode >> i) & 1);
    printf(" - all 32 bits\n");

    //printf("\nMode          : %d\n", buf.st_mode);

    unsigned int entry_mask = 511; // 0000 0000 0000 0000 0000 0001 1111 1111 - взять первых 9 битов

    unsigned int rights = st_mode & entry_mask;
    for (int i = 31; i >= 0; i--)
        printf("%d", (rights >> i) & 1);

    printf(" - after nullification of 1-23 bits\n");

    //printPrava(rights);

    int counter = 0;
    unsigned int ugo_mask = 0;

    while (str[counter] != '+' && str[counter] != '-' && str[counter] != '=')
    {
        switch (str[counter])
        {
        case 'u':
            ugo_mask = ugo_mask | 0700;
            break;
        case 'g':
            ugo_mask = ugo_mask | 0070;
            break;
        case 'o':
            ugo_mask = ugo_mask | 0007;
            break;
        default:
            fprintf(stderr, "Error: no such operation '%c'. Available: u, g, o\n", str[counter]);
            exit(EXIT_FAILURE);
            break;
        }
        counter++;
    }

    for (int i = 31; i >= 0; i--)
        printf("%d", (ugo_mask >> i) & 1);

    printf(" - ugo mask\n");

    char sign;
    if (str[counter] == '+' || str[counter] == '-' || str[counter] == '=')
    {
        sign = str[counter];
        counter++;
    }
    else
    {
        fprintf(stderr, "Error: no such operation '%c'. Available: +, -, =\n", str[counter]);
        exit(EXIT_FAILURE);
    }


    unsigned int right_mask = 0;
    while (str[counter] != '\0')
    {
        switch (str[counter])
        {
        case 'r':
            right_mask = right_mask | 0444;
            break;
        case 'w':
            right_mask = right_mask | 0222;
            break;
        case 'x':
            right_mask = right_mask | 0111;
            break;
        default:
            fprintf(stderr, "Error: no such operation '%c'. Available: r, w, x\n", str[counter]);
            exit(EXIT_FAILURE);
            break;
        }
        counter++;
    }

    for (int i = 31; i >= 0; i--)
        printf("%d", (right_mask >> i) & 1);

    printf(" - right mask\n");

    unsigned int ugo_right_mask = 0;
    ugo_right_mask = ugo_mask & right_mask;



    for (int i = 31; i >= 0; i--)
        printf("%d", (ugo_right_mask >> i) & 1);

    printf(" - ugo right mask\n");

    switch (sign)
    {
    case '+':
        rights = rights | ugo_right_mask;
        break;
    case '-':
        rights = rights ^ ugo_right_mask;
        break;
    case '=':
        rights = (rights & ~ugo_mask) | ugo_right_mask;
        break;
    }


    for (int i = 31; i >= 0; i--)
        printf("%d", (rights >> i) & 1);

    printf(" - final 9 bits\n");



    rights = (st_mode & ~entry_mask) + rights; // возвращаем остальные биты, которые в начале убрали, чтобы не думать о них

    for (int i = 31; i >= 0; i--)
        printf("%d", (rights >> i) & 1);

    printf(" - final 32 bits\n");

    return rights;

}

unsigned int chmod(unsigned int st_mode, char* str)
{
    unsigned int entry_mask = 511; // 0000 0000 0000 0000 0000 0001 1111 1111 - взять первых 9 битов
    unsigned int rights = st_mode & entry_mask;
    int counter = 0;
    unsigned int ugo_mask = 0;

    while (str[counter] != '+' && str[counter] != '-' && str[counter] != '=')
    {
        switch (str[counter])
        {
        case 'u':
            ugo_mask = ugo_mask | 0700;
            break;
        case 'g':
            ugo_mask = ugo_mask | 0070;
            break;
        case 'o':
            ugo_mask = ugo_mask | 0007;
            break;
        default:
            fprintf(stderr, "Error: no such operation '%c'. Available: u, g, o\n", str[counter]);
            exit(EXIT_FAILURE);
            break;
        }
        counter++;
    }

    char sign;
    if (str[counter] == '+' || str[counter] == '-' || str[counter] == '=')
    {
        sign = str[counter];
        counter++;
    }
    else
    {
        fprintf(stderr, "Error: no such operation '%c'. Available: +, -, =\n", str[counter]);
        exit(EXIT_FAILURE);
    }

    unsigned int right_mask = 0;
    while (str[counter] != '\0')
    {
        switch (str[counter])
        {
        case 'r':
            right_mask = right_mask | 0444;
            break;
        case 'w':
            right_mask = right_mask | 0222;
            break;
        case 'x':
            right_mask = right_mask | 0111;
            break;
        default:
            fprintf(stderr, "Error: no such operation '%c'. Available: r, w, x\n", str[counter]);
            exit(EXIT_FAILURE);
            break;
        }
        counter++;
    }

    unsigned int ugo_right_mask = 0;
    ugo_right_mask = ugo_mask & right_mask;

    switch (sign)
    {
    case '+':
        rights = rights | ugo_right_mask;
        break;
    case '-':
        rights = rights ^ ugo_right_mask;
        break;
    case '=':
        rights = (rights & ~ugo_mask) | ugo_right_mask;
        break;
    }

    rights = (st_mode & ~entry_mask) + rights; // возвращаем остальные биты, которые в начале убрали, чтобы не думать о них

    return rights;
}

/*
int main()
{
	struct _stat buf;
	int result;
    char timebuf[26];
	char* filename = "TypicalFile.txt";
	errno_t err;

	result = _stat(filename, &buf);

    
    if (result != 0)
    {
        perror("Problem getting information");
        switch (errno)
        {
        case ENOENT:
            printf("File %s not found.\n", filename);
            break;
        case EINVAL:
            printf("Invalid parameter to _stat.\n");
            break;
        default:
            // Should never be reached. 
            printf("Unexpected error in _stat.\n");
        }
    }
    else
    {
        // Output some of the statistics:
        printf("File size     : %ld\n", buf.st_size);
        printf("Drive         : %c:\n", buf.st_dev + 'A');
        err = ctime_s(timebuf, 26, &buf.st_mtime);
        if (err)
        {
            printf("Invalid arguments to ctime_s.");
            exit(1);
        }
        printf("Time modified : %s", timebuf);
    }

    //printf("\nMode          : %d\n", buf.st_mode);
    

    unsigned int res = 0;

    printf("uo+rwx\n");
    res = chmod(buf.st_mode, (char*)"uo+rwx");

    for (int i = 31; i >= 0; i--)
        printf("%d", (buf.st_mode >> i) & 1);
    printf(" - before uo+rwx\n");
    for (int i = 31; i >= 0; i--)
        printf("%d", (res >> i) & 1);
    printf(" - after uo+rwx\n");

    printf("\ng-w\n");
    res = chmod(buf.st_mode, (char*)"g-w");

    for (int i = 31; i >= 0; i--)
        printf("%d", (buf.st_mode >> i) & 1);
    printf(" - before g-w\n");
    for (int i = 31; i >= 0; i--)
        printf("%d", (res >> i) & 1);
    printf(" - after g-w\n");

    printf("\nu=x\n");
    res = chmod(buf.st_mode, (char*)"u=x");

    for (int i = 31; i >= 0; i--)
        printf("%d", (buf.st_mode >> i) & 1);
    printf(" - before u=x\n");
    for (int i = 31; i >= 0; i--)
        printf("%d", (res >> i) & 1);
    printf(" - after u=x\n");

    printPrava8richnoe(res);
    printPrava2ichnoe(res);
    printPravaBukvami(res);
    
    char* str;


	return 0;
}
*/

unsigned int p1()
{
    char str[100];
    unsigned int res = 0;

    printf("Введите права в буквенном виде: ");
    fgets(str, sizeof(str), stdin);
    for (int i = 0; i < 9; i++)
    {
        res = res * 2;
        if (str[i] != '-')
            res++;
    }

    for (int i = 8; i >= 0; i--)
        printf("%d", (res >> i) & 1);
    printf("\n");
    return res;
}

unsigned int p2()
{
    unsigned int input = 0;

    printf("Введите права в цифровом виде: ");
    scanf_s("%o", &input);

    for (int i = 8; i >= 0; i--)
        printf("%d", (input >> i) & 1);
    printf("\n");
    return input;
}

unsigned int p3()
{
    unsigned int res = 0;
    struct _stat buf;
    int result;
    char filename[100];
    errno_t err;

    printf("Введите имя файла: ");
    fgets(filename, sizeof(filename), stdin);
    for (int i = 0; i < sizeof(filename); i++)
        if (filename[i] == '\n')
            filename[i] = '\0';

    result = _stat(filename, &buf);


    if (result != 0)
    {
        perror("Problem getting information");
        switch (errno)
        {
        case ENOENT:
            printf("File %s not found.\n", filename);
            break;
        case EINVAL:
            printf("Invalid parameter to _stat.\n");
            break;
        default:
            // Should never be reached. 
            printf("Unexpected error in _stat.\n");
        }
    }
    else
    {
        res = buf.st_mode;
        printPravaBukvami(res);
        printPrava8richnoe(res);
        printPrava2ichnoe(res);
    }

    return res;
}

unsigned int p4(unsigned int res)
{
    unsigned int result = 0;
    char text[100];
    printf("Введите права: ");
    fgets(text, sizeof(text), stdin);
    for (int i = 0; i < sizeof(text); i++)
        if (text[i] == '\n')
            text[i] = '\0';
    result = chmod(res, text);
    printPrava2ichnoe(result);
    return result;
}

int main()
{
    setlocale(LC_ALL, "RUS");

    printf("Добро пожаловать в меню программы калькулятора.\n\n");
    int choice = 0;
    unsigned int res = 0;
    char buffer[100];
    while (choice != -1)
    {
        printf("Вам доступны следующие опции для ввода:\n");
        printf("1 - ввести права в буквенном виде, получить в бинарном;\n");
        printf("2 - ввести права в цифровом виде, получить в бинарном;\n");
        printf("3 - ввести имя файла, получить его буквенное, цифровое и битовое представление прав доступа;\n");
        printf("4 - изменить права доступа, введённые в последнем из пунктов 1-3;\n");
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
                    res = p1();
                    break;
                case 2:
                    res = p2();
                    break;
                case 3:
                    res = p3();
                    break;
                case 4:
                    res = p4(res);
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