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

void signal_handler(int sig)
{
    printf("\nЗавершение работы...\n");
    running = 0;
}

int main()
{
    mqd_t mq1, mq2;
    struct mq_attr attr;
    char buffer[MAX_MSG_SIZE];
    unsigned int priority;
    
    signal(SIGINT, signal_handler);
    
    // Настраиваем атрибуты очереди
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSGS;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    
    // Открываем очереди (должны быть уже созданы программой 1)
    mq1 = mq_open(QUEUE1, O_RDWR, 0600, &attr);
    mq2 = mq_open(QUEUE2, O_RDWR, 0600, &attr);
    
    if (mq1 == (mqd_t)-1 || mq2 == (mqd_t)-1) {
        perror("Ошибка открытия очередей");
        exit(1);
    }
    
    printf("=== Программа 2 запущена ===\n");
    printf("Очереди: %s (прием), %s (отправка)\n", QUEUE1, QUEUE2);
    printf("Ожидание сообщения...\n\n");
    
    while (running)
    {
        // Принимаем сообщение из первой очереди
        ssize_t bytes_read = mq_receive(mq1, buffer, MAX_MSG_SIZE, &priority);
        
        if (bytes_read >= 0)
        {
            buffer[bytes_read] = '\0';
            
            // Проверяем сообщение о завершении
            if (priority == EXIT_PRIORITY || strcmp(buffer, "EXIT") == 0)
            {
                printf("Первая программа завершила работу\n");
                running = 0;
                break;
            }
            
            printf("Программа 1: %s (приоритет: %u)\n", buffer, priority);
            
            // Запрашиваем ответ
            printf("Введите ответ: ");
            fflush(stdout);
            
            // Чтение ответа пользователя
            if (fgets(buffer, MAX_MSG_SIZE, stdin) != NULL)
            {
                buffer[strcspn(buffer, "\n")] = 0;
                
                if (strlen(buffer) == 0) continue;
                
                if (strcmp(buffer, "exit") == 0)
                {
                    mq_send(mq2, "EXIT", 5, EXIT_PRIORITY);
                    running = 0;
                    break;
                }
                
                // Отправляем ответ во вторую очередь
                if (mq_send(mq2, buffer, strlen(buffer) + 1, 1) == -1)
                {
                    perror("Ошибка отправки сообщения");
                    break;
                }
                
                printf("Отправлено: %s\n", buffer);
                printf("Ожидание сообщения...\n");
            }
        }
        else if (errno != EAGAIN)
        {
            perror("Ошибка приема сообщения");
            break;
        }
        
        usleep(100000); // Небольшая задержка
    }
    
    // Закрываем очереди
    mq_close(mq1);
    mq_close(mq2);
    
    printf("Программа 2 завершена.\n");
    return 0;
}
