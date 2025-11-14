#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <locale.h>

const int MaxPeople = 50;
int FreeID = 3;
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

typedef struct btree
{
    Person* p;
    struct btree* left, * right;
} btree;

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

bool AddPerson(btree** tree, Person* p)
{
    // Если дерево пустое - создаем корневой узел
    if (*tree == NULL)
    {
        btree* new_node = (btree*)malloc(sizeof(btree));
        if (new_node == NULL)
            return false;

        new_node->p = p;  // Сохраняем указатель на Person
        new_node->left = NULL;
        new_node->right = NULL;
        *tree = new_node;
        return true;
    }

    btree* tmp = *tree;
    btree* prev = NULL;

    while (tmp != NULL)
    {
        prev = tmp;

        if (p->ID < tmp->p->ID)
        {
            tmp = tmp->left;
            // Когда дошли до конца - создаем новый узел слева
            if (tmp == NULL)
            {
                btree* new_node = (btree*)malloc(sizeof(btree));
                if (new_node == NULL)
                    return false;

                new_node->p = p;
                new_node->left = NULL;
                new_node->right = NULL;
                prev->left = new_node;
                return true;
            }
        }
        else if (p->ID > tmp->p->ID)
        {  // Исправлено: было <, теперь >
            tmp = tmp->right;
            // Когда дошли до конца - создаем новый узел справа
            if (tmp == NULL)
            {
                btree* new_node = (btree*)malloc(sizeof(btree));
                if (new_node == NULL)
                    return false;

                new_node->p = p;
                new_node->left = NULL;
                new_node->right = NULL;
                prev->right = new_node;
                return true;
            }
        }
        else
            return false;
    }
    return false;
}

bool Delete(int key, btree** node)
{
    btree* t, * up;
    if (*node == NULL)
        return false; // нет такого значения в дереве
    if ((*node)->p->ID == key)
    {
        // если значение находится в листе, то удаляем лист
        if (((*node)->left == NULL) && ((*node)->right == NULL))
        {
            free(*node);
            *node = NULL;
            printf("Delete List\n");
            return true;
        }
        // если у вершины только правый потомок, то перебрасываем
        // связь на вершину ниже удаляемой в правом поддереве
        if ((*node)->left == NULL)
        {
            t = *node;
            *node = (*node)->right;
            free(t);
            printf("Delete Left = 0\n");
            return 1;
        }
        // если у вершины только левый потомок, то перебрасываем
        // связь на вершину ниже удаляемой в левом поддереве
        if ((*node)->right == NULL)
        {
            t = *node;
            *node = (*node)->left;
            free(t);
            printf("Delete Right = 0\n");
            return 1;
        }
        // если у вершины два потомка, то заменяем удаляемое значение
        // на значение самого правого элемента в левом поддереве
        up = *node;
        t = (*node)->left; // идем в левое поддерево
        //спускаемся до крайнего правого узла
        while (t->right != NULL)
        {
            up = t;
            t = t->right;
        }
        //копируем значение из правого узла вместо удаляемого значения
        //(*node)->p->ID = t->p->ID;
        (*node)->p = t->p;
        // если ниже удаляемого больше, чем одна вершина
        if (up != (*node))
        {
            // если крайний правый не лист, то «отбрасываем» его из дерева
            if (t->left != NULL) up->right = t->left;
            else up->right = NULL; // удаляем лист
        }
        // если ниже удаляемого одна вершина, удаляем лист
        else (*node)->left = t->left;
        // освобождаем память – удаляем крайнюю
        // правую вершину
        free(t);
        printf("Delete Two\n");
        return 1;
    }
    // поиск ключа в левом или правом поддереве
    if ((*node)->p->ID < key)
        return Delete(key, &(*node)->right);
    return Delete(key, &(*node)->left);
}

/*
// Печать содержимого дерева
void Print_Btree(btree* p)
{
    if (p == NULL)
        return;
    Print_Btree(p->left);
    printf("%d ", p->p->ID);
    Print_Btree(p->right);
}
*/

void printTree(btree* root, int level)
{
    if (root == NULL) return;

    // Отступы перед выводом каждого узла
    for (int i = 0; i < level; ++i)
        printf("   ");

    // Печать информации о персонаже
    printf("%d (%s %s)\n", root->p->ID, root->p->FirstName, root->p->LastName);

    // Левое поддерево идет ниже текущего уровня
    printTree(root->left, level + 1);

    // Правое поддерево также идет ниже текущего уровня
    printTree(root->right, level + 1);
}

// Функция для проверки существования ID в дереве
bool isIdExists(btree* tree, int id)
{
    if (tree == NULL)
        return false;
    if (tree->p->ID == id)
        return true;
    return isIdExists(tree->left, id) || isIdExists(tree->right, id);
}

void updatePersonId(btree** tree, Person* p, int new_id)
{
    if (tree == NULL || p == NULL)
    {
        printf("Error: NULL parameter\n");
        return;
    }

    // Сохраняем старый ID
    int old_id = p->ID;

    printf("Trying to change ID from %d to %d\n", old_id, new_id);

    // Проверяем, что нового ID нет в дереве
    if (isIdExists(*tree, new_id))
    {
        printf("Error: ID %d already exists in tree\n", new_id);
        return;
    }

    // Удаляем Person из дерева по старому ID
    //printf("Дерево до удаления:\n");
    //printTree(*tree, 0);  // Убрал & - передаем указатель, а не адрес указателя

    Delete(old_id, tree);

    //printf("Дерево после удаления:\n");
    //printTree(*tree, 0);  // Убрал & - передаем указатель, а не адрес указателя

    // Меняем ID у Person
    p->ID = new_id;
    if (new_id > FreeID)
        FreeID = new_id + 1;

    // Вставляем обратно в дерево с новым ID
    AddPerson(tree, p);  // Убрал & - tree уже является btree**

    printf("ID seccessfully changed from %d to %d\n", old_id, new_id);
}

int main()
{
    setlocale(LC_ALL, "Rus");

    Person p1, p2, p3, p4, p5;
    btree* tree = NULL;
    int firstID = FreeID;

    PersonInit(&p1, (char*)"Ivan", (char*)"Suslikov", 3, (char*)"%tNickTG", (char*)"%pCEO", (char*)"%vNickVK");
    // p1->ID = 1 + 2
    PersonInit(&p2, (char*)"Alexander", (char*)"Samosuslivitze", 1, (char*)"%pPovar");
    // p2->ID = 2 + 2
    PersonInit(&p3, (char*)"Valeriy", (char*)"Kulyebyaka", 1, (char*)"%vProfileVK");
    // p3->ID = 3 + 2
    PersonInit(&p4, (char*)"Alexander", (char*)"AAA", 1, (char*)"%pPovar");
    // p2->ID = 4 + 2
    PersonInit(&p5, (char*)"Valeriy", (char*)"BBB", 1, (char*)"%vProfileVK");
    // p3->ID = 5 + 2

    AddPerson(&tree, &p2);
    AddPerson(&tree, &p1);
    AddPerson(&tree, &p3);
    AddPerson(&tree, &p4);
    AddPerson(&tree, &p5);

    printTree(tree, 0);

    int choice = 0;
    int result;
    while (choice < firstID || choice > FreeID - 1)
    {
        printf_s("Type %d-%d to change someone's ID: ", firstID, FreeID - 1);
        result = scanf_s("%d", &choice);

        if (result != 1)
        {
            while (getchar() != '\n');
            choice = 0;
        }
    }

    int new_id = 0;
    while (new_id == 0)
    {
        printf_s("Type new ID (not equal 0) : ");
        result = scanf_s("%d", &new_id);

        if (result != 1)
        {
            while (getchar() != '\n');
            new_id = 0;
        }
    }

    switch (choice)
    {
    case 3:
        updatePersonId(&tree, &p1, new_id);
        break;
    case 4:
        updatePersonId(&tree, &p2, new_id);
        break;
    case 5:
        updatePersonId(&tree, &p3, new_id);
        break;
    case 6:
        updatePersonId(&tree, &p4, new_id);
        break;
    case 7:
        updatePersonId(&tree, &p5, new_id);
        break;
    }

    //updatePersonId(&tree, &p2, 6);
    printf("\nAt the end we got this beauty:\n");
    printTree(tree, 0);

    printf_s("Type anything to quit: ");
    result = scanf_s("%d", &new_id);

    return 0;
}