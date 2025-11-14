#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//typedef struct Person Person;

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
    // Вернём true, если успешно записали
    // Вернём false, если места не оказалось

    if (*head == NULL)
    {
        *head = p;
        return true;
    }
    Person* tmp = *head;

    while (tmp->next != NULL && tmp->ID < tmp->next->ID)
        tmp = tmp->next;

    if (tmp->ID > p->ID) // вставка в начало
    {
        p->prev = tmp->prev;
        p->next = tmp;
        if (tmp->prev != NULL)
            tmp->prev->next = p;
        else
            *head = p;
        tmp->prev = p;
    }
    else if (tmp->ID < p->ID)
    {
        p->prev = tmp;
        tmp->next = p;
    }
    else
        return false;

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
    // Редактирование записи о человеке по его номеру
    // Редактировать можно только 1 атрибут за раз
    // Хотя забавнее было бы кидать 1 для смены имени, 10 для смены фамилии, 100 для смены должности и тд (и всё это в битах, как права в Линуксе, типо 101 кинуть или 111, чтобы всё сменить)
    // ppl - массив, в котором редактируем человека
    // str - строка с новым атрибутом
    // ID - ID       кто бы мог подумать
    // choice - выбор, что меняем: 1 - имя, 2 - фамилию, 3 - должность, 4 - tg, 5 - vk
    // Вернём true, если успешно отредактировали
    // Вернём false, если возникла какая-либо ошибка. Да, они не будут отличаться, и я даже не поднимаю exception :с

    if (strlen(str) > 20)
        return false;

    Person* tmp = *head;
    while (tmp->ID != p->ID && tmp != NULL)
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

bool ChangeID(Person** head, int from, int to)
{

    Person* tmp = *head;
    while (tmp->ID != from && tmp != NULL)
        tmp = tmp->next;
    
    // если все перебрали, а ID не нашли, то ловим -вайб и возвращаем false
    if (tmp == NULL)
        return false;
    
    // если нашли, причём его prev == NULL, то нужно голову менять (предположительно, мб после смены снова менять её на того же человека, но с этим справится AddPerson)
    if (tmp->prev == NULL)
    {
        *head = tmp->next;
        tmp->ID = to;
        if (tmp->next != NULL)
            tmp->next->prev = NULL;
        tmp->next = NULL;
        AddPerson(tmp,head);
        return true;
    }
    else // тут всё просто, даже голову переставлять не надо. Только аккуратно переставить указатели
    {
        tmp->prev->next = tmp->next;
        if (tmp->next != NULL)
            tmp->next->prev = tmp->prev;
        tmp->next = NULL;
        tmp->prev = NULL;
        tmp->ID = to;
        AddPerson(tmp, head);
        return true;
    }

    // невозможное событие, но да ладно
    return false;
}

int main()
{
    Person p1, p2, p3;
    Person* head = NULL;

    PersonInit(&p1, (char*)"Ivan", (char*)"Suslikov", 3, (char*)"%tNickTG", (char*)"%pCEO", (char*)"%vNickVK");
    // p1->ID = 1
    PersonInit(&p2, (char*)"Alexander", (char*)"Samosuslivitze", 1, (char*)"%pPovar");
    // p2->ID = 2
    PersonInit(&p3, (char*)"Valeriy", (char*)"Kulyebyaka", 1, (char*)"%vProfileVK");
    // p3->ID = 3

    printf_s("At the beginning there are 3 people:\n");
    AddPerson(&p2, &head);
    AddPerson(&p1, &head);
    AddPerson(&p3, &head);
    PrintPeople(head);


    int choice = 0;
    int result;
    while (choice < 1 || choice > 3)
    {
        printf_s("Type 1, 2 or 3 to change someone's ID: ");
        result = scanf_s("%d", &choice);

        if (result != 1)
        {
            while (getchar() != '\n');
            choice = 0;
        }
    }

    /*
    PrintPeople(head);
    DeletePerson(&p1, &head);  // топ смешных ошибок. Забыл менять head, если удаляю первый элемент
    PrintPeople(head);         // поэтому PrintPeople не вывел ничего :)
    EditPerson(&head, (char*)"ANOTHERNAME", &p2, 1);
    PrintPeople(head);
    */

    //DeletePerson(&p3, &head);
    ChangeID(&head, choice, FreeID);
    FreeID++;
    PrintPeople(head);


    printf_s("Type anything to exit.\n");
    result = scanf_s("%d", &choice);
    return 0;
}