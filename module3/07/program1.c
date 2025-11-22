#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>

#define QUEUE1 "/chat_queue1"
#define QUEUE2 "/chat_queue2" 
#define MAX_MSG_SIZE 256
#define MAX_MSGS 10
#define EXIT_PRIORITY 999

int running = 1;

// Обработчик сигналов для корректного завершения
void signal_handler(int sig)
{
    printf("\nЗавершение работы...\n");
    running = 0;
}

int main()
{
    mqd_t mq1, mq2;            // Дескрипторы двух очередей сообщений
    struct mq_attr attr;       // Структура для хранения атрибутов очереди
    char buffer[MAX_MSG_SIZE]; // Буфер для хранения принимаемых/отправляемых сообщений
    unsigned int priority;     // Переменная для хранения приоритета полученного сообщения
    
    // Устанавливаем обработчик сигналов
    signal(SIGINT, signal_handler);
    
    // Настраиваем атрибуты очереди
    attr.mq_flags = 0;              // Флаги: 0 - блокирующий режим (по умолчанию)
    attr.mq_maxmsg = MAX_MSGS;      // Максимальное количество сообщений в очереди (10)
    attr.mq_msgsize = MAX_MSG_SIZE; // Максимальный размер одного сообщения в байтах (256)
    attr.mq_curmsgs = 0;            // Текущее количество сообщений в очереди (инициализация)
    
    // Создаем/открываем очереди
    mq1 = mq_open(QUEUE1, O_CREAT | O_RDWR, 0600, &attr);
    mq2 = mq_open(QUEUE2, O_CREAT | O_RDWR, 0600, &attr);
    
    if (mq1 == (mqd_t)-1 || mq2 == (mqd_t)-1)
    {
        perror("Ошибка создания очередей");
        exit(1);
    }
    
    printf("=== Программа 1 запущена ===\n");
    printf("Очереди: %s (отправка), %s (прием)\n", QUEUE1, QUEUE2);
    printf("Для выхода введите 'exit'\n\n");
    
    // Первая программа начинает диалог
    printf("Введите первое сообщение: ");
    fflush(stdout);
    
    while (running)
    {
        // Чтение ввода пользователя
        if (fgets(buffer, MAX_MSG_SIZE, stdin) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = 0; // Убираем символ новой строки
            
            if (strlen(buffer) == 0)
                continue;
            
            // Проверяем команду выхода
            if (strcmp(buffer, "exit") == 0)
            {
                // Отправляем сообщение о завершении с высоким приоритетом
                mq_send(mq1, "EXIT", 5, EXIT_PRIORITY);
                running = 0;
                break;
            }
            
            // Отправляем сообщение во вторую очередь
            if (mq_send(mq1, buffer, strlen(buffer) + 1, 1) == -1)
            {
                perror("Ошибка отправки сообщения");
                break;
            }
            
            printf("Отправлено: %s\n", buffer);
        }
        
        // Принимаем ответ из первой очереди
        ssize_t bytes_read = mq_receive(mq2, buffer, MAX_MSG_SIZE, &priority);
        
        if (bytes_read >= 0)
        {
            buffer[bytes_read] = '\0';
            
            // Проверяем сообщение о завершении
            if (priority == EXIT_PRIORITY || strcmp(buffer, "EXIT") == 0)
            {
                printf("Вторая программа завершила работу\n");
                running = 0;
                break;
            }
            
            printf("Программа 2: %s (приоритет: %u)\n", buffer, priority);
            
            // Запрашиваем следующий ввод
            printf("Введите ответ: ");
            fflush(stdout);
        }
        else if (errno != EAGAIN)
        {
            perror("Ошибка приема сообщения");
            break;
        }
    }
    
    // Закрываем и удаляем очереди
    mq_close(mq1);
    mq_close(mq2);
    mq_unlink(QUEUE1);
    mq_unlink(QUEUE2);
    
    printf("Программа 1 завершена.\n");
    return 0;
}
