#include <iostream>
#include <stdarg.h>

const int MaxPeople = 50;
int FreeID = 1;
int PeopleAmount = 0;

typedef struct {

    char tg[20];
    char vk[20];

} SocialNetwork;

typedef struct {

    // Объявляю поля структуры: id, имя, фамилия, должность на работе, структура соцсетей
    int ID;
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

    strncpy_s(p->FirstName, Firstname, 20);
    strncpy_s(p->LastName, Lastname, 20);

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
            strncpy_s(p->SW.tg, tmpstring + 2, 20);
            break;
        case 'v':
            strncpy_s(p->SW.vk, tmpstring + 2, 20);
            break;
        case 'p':
            strncpy_s(p->Position, tmpstring + 2, 20);
            break;
        }
    }

    va_end(factor);
    return true;
}

bool AddPerson(Person* p, Person* ppl)
{
    // Вернём true, если успешно записали
    // Вернём false, если места не оказалось

    if (FreeID == MaxPeople)
        return false;
    ppl[PeopleAmount].ID = FreeID;
    strncpy_s(ppl[PeopleAmount].FirstName, p->FirstName, 20);
    strncpy_s(ppl[PeopleAmount].LastName, p->LastName, 20);
    strncpy_s(ppl[PeopleAmount].Position, p->Position, 20);
    strncpy_s(ppl[PeopleAmount].SW.tg, p->SW.tg, 20);
    strncpy_s(ppl[PeopleAmount].SW.vk, p->SW.vk, 20);

    FreeID++;
    PeopleAmount++;

    return true;
}

int GetID(Person* p, Person* ppl)
{
    // Вернём ID, если успешно нашли такого человека
    // Вернём -1, если такого челвоека нет

    for (int i = 0; i < PeopleAmount; i++)
    {
        if (strcmp(ppl[i].FirstName, p->FirstName) == 0 && strcmp(ppl[i].LastName, p->LastName) == 0)
            return ppl[i].ID;
    }

    return -1;
}

bool DeletePerson(Person* p, Person* ppl)
{
    // Вернём true, если успешно удалили
    // Вернём false, если не нашли такого человека

    int ID = GetID(p, ppl);
    for (int i = 0; i < PeopleAmount; i++)
    {
        if (ppl[i].ID == ID)
        {
            PeopleAmount--;
            for (int j = i; j < PeopleAmount; j++)
                ppl[j] = ppl[j + 1];

            return true;
        }
    }

    return false;
}

bool EditPerson(Person* ppl, char* str, int ID,int choice)
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

    if (ID <= 0 || ID >= FreeID)
        return false;

    int number = -1;
    for (int i = 0; i < PeopleAmount; i++)
        if (ppl[i].ID == ID)
            number = i;

    if (number == -1)
        return false;

    switch(choice)
    {
    case 1:
        strncpy_s(ppl[number].FirstName, str, 20);
        break;
    case 2:
        strncpy_s(ppl[number].LastName, str, 20);
        break;
    case 3:
        strncpy_s(ppl[number].Position, str, 20);
        break;
    case 4:
        strncpy_s(ppl[number].SW.tg, str, 20);
        break;
    case 5:
        strncpy_s(ppl[number].SW.vk, str, 20);
        break;
    default:
        return false;
    }
    return true;
}

void PrintPeople(Person* ppl)
{
    // функция для тестирования, показывающая весь массив
    // Я помню, что нельзя привязываться к конкретному std::out. Но это тестовая вещь, в задании вообще не сказано её делать (Хотя без неё не понять, какую запись редактировать)

    printf_s("\nNo ID FirstName            LastName             Position             tg                   vk\n\n");
    for (int i = 0; i < PeopleAmount; i++)
    {
        if (ppl[i].FirstName[0] != '\0')
        {
            printf_s("%d) %-2d %-20s %-20s %-20s %-20s %-20s\n", i, ppl[i].ID, ppl[i].FirstName, ppl[i].LastName, ppl[i].Position, ppl[i].SW.tg, ppl[i].SW.vk);
        }
    }
    printf_s("\n");
}

int main()
{
    Person p1{}, p2{}, p3{};
    Person ppl[MaxPeople]{};

    PersonInit(&p1, (char*)"Ivan", (char*)"Suslikov", 3, (char*)"%tNickTG", (char*)"%pCEO", (char*)"%vNickVK");
    //printf("%s %s %s %s %s\n", p1.FirstName, p1.LastName, p1.Position, p1.SW.tg, p1.SW.vk);
    AddPerson(&p1, ppl);

    PersonInit(&p2, (char*)"Alexander", (char*)"Samosuslivitze", 1, (char*)"%pPovar");
    //printf("%s %s %s\n", p2.FirstName, p2.LastName, p2.Position);
    AddPerson(&p2, ppl);

    PrintPeople(ppl);

    DeletePerson(&p2, ppl);

    PersonInit(&p3, (char*)"Valeriy", (char*)"Kulyebyaka", 1, (char*)"%vProfileVK");
    //printf("%s %s\n", p3.FirstName, p3.LastName);
    AddPerson(&p3, ppl);

    PrintPeople(ppl);

    EditPerson(ppl, (char*)"ProfileTG", GetID(&p3, ppl), 4);

    PrintPeople(ppl);

    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
