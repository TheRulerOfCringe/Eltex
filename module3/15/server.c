#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

// функция обслуживания подключившихся пользователей
// void dostuff(int sock);

// функция обработки ошибок
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// количество активных пользователей
int nclients = 0;

// печать количества активных пользователей
void printusers()
{
    if(nclients)
        printf("%d user on-line\n", nclients);
    else
        printf("No User on line\n");
}

// функция обработки данных 1
int plus(int a, int b)
{
    return a + b;
}

// функция обработки данных 2
int minus(int a, int b)
{
    return a - b;
}

// функция обработки данных 3
int mult(int a, int b)
{
    return a * b;
}

// структура для хранения состояния клиента
typedef struct {
    int sockfd;
    int state; // 0: ожидание первого параметра, 1: ожидание второго параметра, 2: ожидание операции, 3: отправка результата
    int a, b, op;
    char buffer[1024];
    int buf_pos;
} client_state_t;

#define MAX_CLIENTS 10
client_state_t clients[MAX_CLIENTS];

int main(int argc, char *argv[])
{
    char buff[1024];
    int sockfd, newsockfd;
    int portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    // ошибка в случае если мы не указали порт
    if (argc < 2)
    {
        fprintf(stderr, "Try \"./server port\"\n");
        exit(1);
    }

    // Инициализация клиентов
    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i].sockfd = -1;

    // Шаг 1 - создание сокета
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("socket");
    
    // Установка опции повторного использования адреса
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        error("setsockopt");
    
    // Шаг 2 - связывание сокета с локальным адресом
    bzero((char*) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("binding");
    
    // Шаг 3 - ожидание подключений, размер очереди - 5
    listen(sockfd, 5);
    
    fd_set readfds;
    int max_fd;
    
    printf("Server started on port %d\n", portno);
    
    while (1)
    {
        FD_ZERO(&readfds);
        
        // Добавляем серверный сокет в набор
        FD_SET(sockfd, &readfds);
        max_fd = sockfd;
        
        // Добавляем клиентские сокеты в набор
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].sockfd > 0)
            {
                FD_SET(clients[i].sockfd, &readfds);
                if (clients[i].sockfd > max_fd)
                    max_fd = clients[i].sockfd;
            }
        }
        
        // Ожидаем активности на сокетах
        // max_fd + 1 - количество проверяемых дескрипторов
        // &readfds - набор дескрипторов для чтения
        // NULL - набор для записи
        // NULL - набор для исключений
        // NULL - таймаут (бесконечное ожидание)
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        
        
        if ((activity < 0) && (errno != EINTR))
            perror("select");
        
        // Проверяем новое подключение
        if (FD_ISSET(sockfd, &readfds))
        {
            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if (newsockfd < 0)
            {
                perror("accept");
                continue;
            }
            
            // Находим свободный слот для нового клиента
            int client_index = -1;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].sockfd < 0)
                {
                    client_index = i;
                    break;
                }
            }
            
            if (client_index == -1) {
                printf("Too many clients!\n");
                close(newsockfd);
            }
            else
            {
                // Инициализируем состояние нового клиента
                clients[client_index].sockfd = newsockfd;
                clients[client_index].state = 0;
                clients[client_index].buf_pos = 0;
                memset(clients[client_index].buffer, 0, sizeof(clients[client_index].buffer));
                
                nclients++;
                printf("+ [%s] new connect!\n", inet_ntoa(cli_addr.sin_addr));
                printusers();
                
                // Отправляем первый запрос
                const char *str1 = "Enter 1 parameter\r\n";
                write(newsockfd, str1, strlen(str1));
            }
        }
        
        // Проверяем активность клиентов
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sockfd > 0 && FD_ISSET(clients[i].sockfd, &readfds)) {
                client_state_t *client = &clients[i];
                
                // Читаем данные от клиента
                int bytes_recv = read(client->sockfd, 
                                    client->buffer + client->buf_pos, 
                                    sizeof(client->buffer) - client->buf_pos - 1);
                
                if (bytes_recv <= 0) {
                    // Соединение закрыто или ошибка
                    printf("- [socket %d] disconnect\n", client->sockfd);
                    close(client->sockfd);
                    client->sockfd = -1;
                    nclients--;
                    printusers();
                    continue;
                }
                
                client->buf_pos += bytes_recv;
                client->buffer[client->buf_pos] = '\0';
                
                // Проверяем, есть ли завершающий символ новой строки
                char *newline = strchr(client->buffer, '\n');
                if (newline != NULL) {
                    *newline = '\0'; // Завершаем строку на символе новой строки
                    
                    // Обрабатываем ввод в зависимости от состояния
                    switch(client->state) {
                        case 0: // Ожидание первого параметра
                            client->a = atoi(client->buffer);
                            client->state = 1;
                            client->buf_pos = 0;
                            memset(client->buffer, 0, sizeof(client->buffer));
                            
                            // Запрашиваем второй параметр
                            write(client->sockfd, "Enter 2 parameter\r\n", 19);
                            break;
                            
                        case 1: // Ожидание второго параметра
                            client->b = atoi(client->buffer);
                            client->state = 2;
                            client->buf_pos = 0;
                            memset(client->buffer, 0, sizeof(client->buffer));
                            
                            // Запрашиваем операцию
                            write(client->sockfd, "Enter 1 for +, 2 for -, 3 for * (default is +)\r\n", 49);
                            break;
                            
                        case 2: // Ожидание операции
                            client->op = atoi(client->buffer);
                            client->state = 3;
                            client->buf_pos = 0;
                            memset(client->buffer, 0, sizeof(client->buffer));
                            
                            // Выполняем операцию
                            int result;
                            switch(client->op) {
                                case 2:
                                    result = minus(client->a, client->b);
                                    break;
                                case 3:
                                    result = mult(client->a, client->b);
                                    break;
                                default:
                                    result = plus(client->a, client->b);
                                    break;
                            }
                            
                            // Отправляем результат
                            snprintf(buff, sizeof(buff), "Result: %d\r\n", result);
                            write(client->sockfd, buff, strlen(buff));
                            
                            // Закрываем соединение после отправки результата
                            printf("- [socket %d] operation completed, disconnecting\n", client->sockfd);
                            close(client->sockfd);
                            client->sockfd = -1;
                            nclients--;
                            printusers();
                            break;
                    }
                }
            }
        }
    }
    
    close(sockfd);
    return 0;
}
