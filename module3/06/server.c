#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#define SERVER_TYPE 10
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 256

static int message_counter = 0; // Для отладки и для победы над утечкой памяти диска

// Структура сообщения
struct message
{
    long mtype;
    int sender_id;
    char mtext[MAX_MSG_SIZE];
};

// Структура для хранения информации о клиентах
struct client_info
{
    int active;
    int client_id;
};

int msgq_id;
struct client_info clients[MAX_CLIENTS];

// Функция для инициализации клиентов
void init_clients()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].active = 0;
        clients[i].client_id = 0;
    }
}

// Функция для добавления клиента
int add_client(int client_id)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i].active)
        {
            clients[i].active = 1;
            clients[i].client_id = client_id;
            printf("Клиент %d подключен\n", client_id);
            return 1;
        }
    }
    return 0; // Нет свободного места
}

// Функция для удаления клиента
void remove_client(int client_id)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].active && clients[i].client_id == client_id)
        {
            clients[i].active = 0;
            clients[i].client_id = 0;
            printf("Клиент %d отключен\n", client_id);
            break;
        }
    }
}

// Функция для проверки активности клиента
int is_client_active(int client_id)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].active && clients[i].client_id == client_id)
            return 1;
    }
    return 0;
}

/*
// Функция для рассылки сообщения всем клиентам
void broadcast_message(struct message *msg)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].active && clients[i].client_id != msg->sender_id)
        {
            struct message broadcast_msg;
            broadcast_msg.mtype = clients[i].client_id;
            broadcast_msg.sender_id = msg->sender_id;
            snprintf(broadcast_msg.mtext, MAX_MSG_SIZE, "%s", msg->mtext);
            
            if (msgsnd(msgq_id, &broadcast_msg, sizeof(broadcast_msg) - sizeof(long), 0) == -1)
                perror("Ошибка отправки сообщения клиенту");
        }
    }
}
*/

void broadcast_message(struct message *msg)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].active && clients[i].client_id != msg->sender_id)
        {
            struct message broadcast_msg;
            broadcast_msg.mtype = clients[i].client_id;  // Отправляем на ID клиента
            broadcast_msg.sender_id = msg->sender_id;
            snprintf(broadcast_msg.mtext, MAX_MSG_SIZE, "%s", msg->mtext);
            
            printf("Отправка сообщения клиенту %d: %s\n", clients[i].client_id, msg->mtext);
            
            if (msgsnd(msgq_id, &broadcast_msg, sizeof(broadcast_msg) - sizeof(long), 0) == -1)
                perror("Ошибка отправки сообщения клиенту");
        }
    }
}

// Обработчик сигналов для корректного завершения
void signal_handler(int sig)
{
    printf("\nЗавершение работы сервера...\n");
    
    // Отправляем сообщение о завершении всем клиентам
    struct message shutdown_msg;
    shutdown_msg.mtype = 1; // Условно инициализируем, потом подставим нужное значение
    shutdown_msg.sender_id = SERVER_TYPE;
    snprintf(shutdown_msg.mtext, MAX_MSG_SIZE, "SERVER_SHUTDOWN");
    
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].active)
        {
            shutdown_msg.mtype = clients[i].client_id;
            msgsnd(msgq_id, &shutdown_msg, sizeof(shutdown_msg) - sizeof(long), IPC_NOWAIT); // Кстати, отнимаем sizeof(long), потому что принимающий ждёт резмер без type, а type у нас long
        }
    }
    
    // Удаляем очередь сообщений
    msgctl(msgq_id, IPC_RMID, NULL);
    exit(0);
}

int main()
{
    key_t key;
    struct message msg;
    
    // Устанавливаем обработчик сигналов
    signal(SIGINT, signal_handler);
    
    // Создаем ключ для очереди сообщений
    if ((key = ftok(".", 'A')) == -1)
    {
        perror("Ошибка создания ключа");
        exit(1);
    }
    printf("key = %d\n", key);
    
    // Создаем очередь сообщений
    if ((msgq_id = msgget(key, IPC_CREAT | 0666)) == -1)
    {
        perror("Ошибка создания очереди сообщений");
        exit(1);
    }
    
    printf("Сервер запущен. ID очереди: %d\n", msgq_id);
    printf("Ожидание подключения клиентов...\n");
    
    init_clients();
    
    while (1)
    {
        if (msgrcv(msgq_id, &msg, sizeof(msg) - sizeof(long), -10, 0) == -1)
        {
            perror("msgrcv");
            continue;
        }

        printf("Получено сообщение типа %ld от клиента %d: %s\n", msg.mtype, msg.sender_id, msg.mtext);

        // Анализируем тип полученного сообщения
        if (msg.mtype == 1)
        {
            // Обработка сообщений типа 1 (управляющие сообщения)
            if (strcmp(msg.mtext, "connect") == 0)
            {
                if (!add_client(msg.sender_id))
                    printf("Достигнуто максимальное количество клиентов\n");
            }
            else if (strcmp(msg.mtext, "shutdown") == 0)
            {
                remove_client(msg.sender_id);
            }
            else
            {
                printf("Неизвестная команда от клиента %d: %s\n", msg.sender_id, msg.mtext);
            }
        }
        else if (msg.mtype == 10)
        {
            // Обработка сообщений типа 10 (пользовательские сообщения)
            message_counter++;
            if (message_counter > 1000)
            {
                printf("ПРЕДУПРЕЖДЕНИЕ: Превышен лимит сообщений (%d)\n", message_counter);
                break;
            }
            
            if (is_client_active(msg.sender_id))
            {
                broadcast_message(&msg);
            }
            else
            {
                printf("Клиент %d не зарегистрирован\n", msg.sender_id);
            }
        }
        else
        {
            printf("Получено сообщение неизвестного типа %ld от клиента %d: %s\n", 
                   msg.mtype, msg.sender_id, msg.mtext);
        }
    }
    
    /*
    // Основной цикл сервера
    while (1)
    {
        //        (id очереди, сообщение, размер без type           , type 0 - ждём первое же сообщение, флаг блокирующий)
        msg.mtype = 11;
        if (msgrcv(msgq_id   , &msg     , sizeof(msg) - sizeof(long), 0                                , 0               ) == -1)
        {
            perror("msgrcv");
            continue;
        }

        // Анализируем тип полученного сообщения
        int choice = 0;
        if (msg.mtype == 1)
            choice = 1;
        else if (msg.mtype == 10)
            choice = 2;
        else if (msg.mtype % 10 == 0)
            choice = 0;
        else
            choice = 0;
        
        switch (choice)
        {
            case 1:
                printf("Получено сообщение типа 1 от клиента %d: %s\n", msg.sender_id, msg.mtext);
                
                // Обработка сообщений типа 1
                if (strcmp(msg.mtext, "connect") == 0)
                {
                    if (!add_client(msg.sender_id)) // Новый клиент подключается
                        printf("Достигнуто максимальное количество клиентов\n");
                }
                else if (strcmp(msg.mtext, "shutdown") == 0)
                    remove_client(msg.sender_id); // Клиент отключается
                else
                    printf("Неизвестная команда от клиента %d: %s\n", msg.sender_id, msg.mtext); // Неизвестная команда типа 1
                break;
                
            case 2:
                message_counter++;
                if (message_counter > 1000)
                {
                    printf("ПРЕДУПРЕЖДЕНИЕ: Превышен лимит сообщений (%d)\n", message_counter);
                    break;
                }
                printf("Получено сообщение типа 10 от клиента %d: %s\n", msg.sender_id, msg.mtext);
                
                // Обработка сообщений типа 10 - пересылаем всем
                if (is_client_active(msg.sender_id))
                    broadcast_message(&msg);
                else
                    printf("Клиент %d не зарегистрирован\n", msg.sender_id);
                break;
                
            default:
                printf("Получено сообщение неизвестного типа %ld от клиента %d: %s\n", msg.mtype, msg.sender_id, msg.mtext);
        }
    }
    */
    
    // Удаляем очередь сообщений
    msgctl(msgq_id, IPC_RMID, NULL);
    printf("Сервер завершил работу.\n");
    
    return 0;
}
