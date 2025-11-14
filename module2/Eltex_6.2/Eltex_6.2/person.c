#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MaxPeople = 50;
int FreeID = 1;
int PeopleAmount = 0;

typedef struct
{
    char tg[20];
    char vk[20];

} SocialNetwork;

typedef struct Person
{
    // Объявляю поля структуры: id, имя, фамилия, должность на работе, структура соцсетей
    int ID;
    struct Person* next;
    struct Person* prev;
    char FirstName[20];
    char LastName[20];
    char Position[20];
    SocialNetwork SW;

} Person;

Person* CreatePerson()
{
    return (Person*)malloc(sizeof(Person));
}

void FreePerson(Person* p)
{
    free(p);
}

bool PersonInit(Person* p, char* Firstname, char* Lastname, int varamount, ...)
{
    // Указатель на стурктуру Person, два обязательных параметра, количество необязательных
    // Пример необязательнго параметра: "%tAnonim1727"
    // %t - tg    %v - vk     %p - position

    p->next = NULL;
    p->prev = NULL;
    p->ID = FreeID;
    FreeID++;
    memset(p->Position, 0, sizeof(p->Position));
    memset(p->SW.tg, 0, sizeof(p->SW.tg));
    memset(p->SW.vk, 0, sizeof(p->SW.vk));

    strncpy_s(p->FirstName, 20, Firstname, 20);
    strncpy_s(p->LastName, 20, Lastname, 20);

    va_list factor;
    va_start(factor, Lastname);
    char* tmpstring;
    char choice;

    for (int i = 0; i < varamount; i++)
    {
        tmpstring = va_arg(factor, char*);
        if (tmpstring[0] != '%')
            return false; // ошибка, если нет %
        choice = tmpstring[1];
        switch (choice)
        {
        case 't':
            strncpy_s(p->SW.tg, 20, tmpstring + 2, 20);
            break;
        case 'v':
            strncpy_s(p->SW.vk, 20, tmpstring + 2, 20);
            break;
        case 'p':
            strncpy_s(p->Position, 20, tmpstring + 2, 20);
            break;
        }
    }

    va_end(factor);
    return true;
}

bool AddPerson(Person* p, Person** head)
{
    if (*head == NULL)
    {
        *head = p;
        p->next = NULL;
        p->prev = NULL;
        return true;
    }

    Person* current = *head;
    Person* prev = NULL;

    while (current != NULL && current->ID < p->ID)
    {
        prev = current;
        current = current->next;
    }

    if (current != NULL && current->ID == p->ID)
    {
        return false;
    }

    p->next = current;
    p->prev = prev;

    if (prev == NULL)
        *head = p;
    else
        prev->next = p;

    if (current != NULL)
        current->prev = p;

    return true;
}

bool DeletePerson(Person* p, Person** head)
{
    Person* tmp = *head;
    while (tmp->ID != p->ID && tmp != NULL)
        tmp = tmp->next;

    if (tmp == NULL)
        return false;
    else
    {
        if ((*head)->ID == tmp->ID)
            *head = tmp->next;

        if (tmp->next != NULL) // без if NULL'у пытаемся дать указатель
            tmp->next->prev = tmp->prev;
        if (tmp->prev != NULL)
            tmp->prev->next = tmp->next;
        memset(tmp, 0, sizeof(Person)); // Я ПРАВИЛЬНО ПОНЯЛ, ЧТО ТАК НАДО ОЧИСТИТЬ?????????????????
        return true;
    }
}

bool EditPerson(Person** head, char* str, Person* p, int choice)
{
    if (str == NULL || strlen(str) > 20)
        return false;

    Person* tmp = *head;

    while (tmp != NULL && tmp->ID != p->ID)
        tmp = tmp->next;

    if (tmp == NULL)
        return false;

    switch (choice)
    {
    case 1:
        strncpy_s(tmp->FirstName, 20, str, 20);
        break;
    case 2:
        strncpy_s(tmp->LastName, 20, str, 20);
        break;
    case 3:
        strncpy_s(tmp->Position, 20, str, 20);
        break;
    case 4:
        strncpy_s(tmp->SW.tg, 20, str, 20);
        break;
    case 5:
        strncpy_s(tmp->SW.vk, 20, str, 20);
        break;
    default:
        return false;
    }
    return true;
}

void PrintPeople(Person* head)
{
    // функция для тестирования, показывающая весь массив
    // Я помню, что нельзя привязываться к конкретному std::out. Но это тестовая вещь, в задании вообще не сказано её делать (Хотя без неё не понять, какую запись редактировать)
    Person* tmp = head;
    printf_s("\nID FirstName            LastName             Position             tg                   vk\n\n");
    while (tmp != NULL)
    {
        printf_s("%-2d %-20s %-20s %-20s %-20s %-20s\n", tmp->ID, tmp->FirstName, tmp->LastName, tmp->Position, tmp->SW.tg, tmp->SW.vk);
        tmp = tmp->next;
    }
    printf_s("\n");
}