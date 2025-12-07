#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#define PORT 12345
#define BUFFER_SIZE 1000

int running = 1;
void signal_handler(int sig)
{
    running = 0;
}

int main()
{
    signal(SIGINT, signal_handler);
    int sockfd;
    int n;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, client1, client2;
    int client1_active = 0, client2_active = 0;
    char client1_name[6] = "user1";
    char client2_name[6] = "user2";
    
    /* Заполняем структуру для адреса сервера */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* Создаем UDP сокет */
    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    
    /* Устанавливаем таймаут на прием данных - 1 секунда */
    struct timeval tv;
    tv.tv_sec = 1;    // 1 секунда
    tv.tv_usec = 0;   // 0 микросекунд
    
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("setsockopt SO_RCVTIMEO failed");
        close(sockfd);
        exit(1);
    }
    
    /* Настраиваем адрес сокета */
    if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }
    
    printf("UDP Chat Server started on port %d\n", PORT);
    printf("Waiting for 2 clients...\n");
    
    while(running)
    {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        
        /* Ожидаем сообщение от клиента */
        if((n = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, (struct sockaddr *) &cliaddr, &clilen)) < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
                continue;
            perror("recvfrom failed");
            continue;
        }
        
        buffer[n] = '\0';
        
        /* Регистрируем клиентов */
        if (!client1_active)
        {
            client1 = cliaddr;
            client1_active = 1;
            printf("%s connected: %s:%d\n", 
                   client1_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        }
        else if
        (!client2_active && (client1.sin_addr.s_addr != cliaddr.sin_addr.s_addr || client1.sin_port != cliaddr.sin_port))
        {
            client2 = cliaddr;
            client2_active = 1;
            printf("%s connected: %s:%d\n", 
                   client2_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            printf("Both clients connected! Chat started.\n");
        }
        
        /* Формируем сообщение с именем отправителя */
        char forward_msg[BUFFER_SIZE];
        if (client1_active && client1.sin_addr.s_addr == cliaddr.sin_addr.s_addr && client1.sin_port == cliaddr.sin_port)
            snprintf(forward_msg, BUFFER_SIZE, "[%s]: %.*s", client1_name, (int)(BUFFER_SIZE - strlen(client1_name) - 5), buffer);
        else if (client2_active)
            snprintf(forward_msg, BUFFER_SIZE, "[%s]: %.*s", client2_name, (int)(BUFFER_SIZE - strlen(client2_name) - 5), buffer);
        else
            continue; // Неизвестный клиент
        
        printf("%s", forward_msg);
        
        /* Пересылаем сообщение другому клиенту */
        if (client1_active && client2_active)
        {
            if (client1.sin_addr.s_addr == cliaddr.sin_addr.s_addr && client1.sin_port == cliaddr.sin_port)
            {
                /* От user1 к user2 */
                sendto(sockfd, forward_msg, strlen(forward_msg), 0,
                      (struct sockaddr *) &client2, sizeof(client2));
            }
            else
            {
                /* От user2 к user1 */
                sendto(sockfd, forward_msg, strlen(forward_msg), 0,
                      (struct sockaddr *) &client1, sizeof(client1));
            }
        }
    }
    
    char exit_msg[] = "Server: shutdown";

    if (client1_active)
    {
        sendto(sockfd, exit_msg, strlen(exit_msg), 0,
              (struct sockaddr *) &client1, sizeof(client1));
    }

    if (client2_active)
    {
        sendto(sockfd, exit_msg, strlen(exit_msg), 0,
              (struct sockaddr *) &client2, sizeof(client2));
    }
    
    close(sockfd);
    return 0;
}
