#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//#pragma comment(lib, "person.lib")

typedef struct Person Person;

Person* CreatePerson();
void FreePerson(Person* p);
bool PersonInit(Person* p, char* Firstname, char* Lastname, int varamount, ...);
bool AddPerson(Person* p, Person** head);
bool DeletePerson(Person* p, Person** head);
bool EditPerson(Person** head, char* str, Person* p, int choice);
void PrintPeople(Person* head);

int main()
{
    Person* p1 = CreatePerson();
    Person* p2 = CreatePerson();
    Person* p3 = CreatePerson();
    Person* head = NULL;

    PersonInit(p1, (char*)"Ivan", (char*)"Suslikov", 3, (char*)"%tNickTG", (char*)"%pCEO", (char*)"%vNickVK");
    // p1->ID = 1
    PersonInit(p2, (char*)"Alexander", (char*)"Samosuslivitze", 1, (char*)"%pPovar");
    // p2->ID = 2
    PersonInit(p3, (char*)"Valeriy", (char*)"Kulyebyaka", 1, (char*)"%vProfileVK");
    // p3->ID = 3

    AddPerson(p2, &head);
    AddPerson(p1, &head);
    AddPerson(p3, &head);
    PrintPeople(head);
    DeletePerson(p1, &head);  // топ смешных ошибок. Забыл менять head, если удаляю первый элемент
    PrintPeople(head);         // поэтому PrintPeople не вывел ничего :)
    EditPerson(&head, (char*)"ANOTHERNAME", p2, 1);
    PrintPeople(head);




    FreePerson(p1);
    FreePerson(p2);
    FreePerson(p3);
    return 0;
}


/*
gcc -fPIC person.c -c
gcc -shared person.o -o person.dll
gcc Main.c person.dll -o program.exe
*/