#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>

#define SERVER_TYPE 10
#define MAX_MSG_SIZE 256

// Структура сообщения
struct message
{
    long mtype;
    int sender_id;
    char mtext[MAX_MSG_SIZE];
};

int msgq_id;
int client_id;
int running = 1;

// Функция для отправки сообщений серверу (тип 1)
void send_to_server(const char *text)
{
    struct message msg;
    msg.mtype = 1; // Сообщения для сервера имеют тип 1
    msg.sender_id = client_id;
    snprintf(msg.mtext, MAX_MSG_SIZE, "%s", text);
    
    if (msgsnd(msgq_id, &msg, sizeof(msg) - sizeof(long), 0) == -1)
        perror("Ошибка отправки сообщения серверу");
}

// Функция для отправки сообщений пользователям (тип 10)
void send_to_users(const char *text)
{
    struct message msg;
    msg.mtype = 10; // Сообщения для пользователей имеют тип 10
    msg.sender_id = client_id;
    snprintf(msg.mtext, MAX_MSG_SIZE, "%s", text);
    
    if (msgsnd(msgq_id, &msg, sizeof(msg) - sizeof(long), 0) == -1)
        perror("Ошибка отправки сообщения пользователям");
}


// Функция для приема сообщений в отдельном потоке
void* receive_messages(void *arg)
{
    struct message msg;
    
    while (running)
    {
        // Получаем сообщения, адресованные этому клиенту
        if (msgrcv(msgq_id, &msg, sizeof(msg) - sizeof(long), client_id, IPC_NOWAIT) != -1)
        {
            if (strcmp(msg.mtext, "SERVER_SHUTDOWN") == 0) {
                printf("\nСервер завершил работу. Выход...\n");
                running = 0;
                break;
            }
            else
            {
                printf("\n[Клиент %d]: %s\n", msg.sender_id, msg.mtext);
                printf("Введите сообщение: ");
                fflush(stdout);
            }
        }
        //usleep(100000); // Небольшая задержка для уменьшения нагрузки на CPU
    }
    return NULL;
}

// Обработчик сигналов
void signal_handler(int sig)
{
    printf("\nЗавершение работы клиента...\n");
    send_to_server("shutdown");
    running = 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Использование: %s <ID_клиента>\n", argv[0]);
        printf("ID клиента должен быть одним из: 20, 30, 40, ...\n");
        exit(1);
    }
    
    client_id = atoi(argv[1]);
    if (client_id < 20 || client_id % 10 != 0)
    {
        printf("Ошибка: ID клиента должен быть одним из: 20, 30, 40, ...\n");
        exit(1);
    }
    
    key_t key;
    
    // Устанавливаем обработчик сигналов
    signal(SIGINT, signal_handler);
    
    // Получаем ключ для очереди сообщений
    if ((key = ftok(".", 'A')) == -1)
    {
        perror("Ошибка создания ключа");
        exit(1);
    }
    
    // Подключаемся к очереди сообщений
    if ((msgq_id = msgget(key, 0666)) == -1)
    {
        perror("Ошибка подключения к очереди сообщений");
        exit(1);
    }
    
    printf("Клиент %d запущен. Подключение к серверу...\n", client_id);
    
    // Регистрируемся на сервере (тип 1)
    send_to_server("connect");
    
    printf("Подключение успешно. Для выхода введите 'exit'\n\n");
    
    // Основной цикл
    char input[MAX_MSG_SIZE];
    while (running)
    {
        // Выводим приглашение
        printf("Введите сообщение: ");
        fflush(stdout);
        
        // Ждем ввод от пользователя
        if (fgets(input, MAX_MSG_SIZE, stdin) == NULL)
            break;
        
        // Убираем символ новой строки
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0)
            continue;
        
        if (strcmp(input, "exit") == 0)
        {
            // Отправляем отключение серверу (тип 1)
            send_to_server("shutdown");
            running = 0;
            break;
        }
        
        // Отправляем сообщение пользователям (тип 10)
        send_to_users(input);
        printf("Отправлено: %s\n", input);
        
        // После отправки проверяем, не пришли ли новые сообщения
            struct message incoming_msg;
    
        // Проверяем все сообщения в очереди (неблокирующий режим)
        while (msgrcv(msgq_id, &incoming_msg, sizeof(incoming_msg) - sizeof(long), client_id, IPC_NOWAIT) != -1)
        {
            if (strcmp(incoming_msg.mtext, "SERVER_SHUTDOWN") == 0)
            {
                printf("\nСервер завершил работу. Выход...\n");
                running = 0;
                break;
            }
            else
                printf("\n[Клиент %d]: %s\n", incoming_msg.sender_id, incoming_msg.mtext);
        }
    }
    
    printf("Клиент %d завершил работу.\n", client_id);
    return 0;
}
