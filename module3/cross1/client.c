#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <signal.h>

#define BUFFER_SIZE 1000
#define SECRET 2082003
#define SECRET_SIZE 9

int running = 1;
void signal_handler(int sig)
{
    running = 0;
}

int main(int argc, char **argv)
{
    signal(SIGINT, signal_handler);
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    fd_set readfds;
    fd_set readfds_copy;
    char sendline[BUFFER_SIZE], recvline[BUFFER_SIZE], temp[BUFFER_SIZE-SECRET_SIZE];
    
    if(argc != 3)
    {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }
    
    /* Создаем UDP сокет */
    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    
    /* Заполняем структуру для адреса клиента */
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = 0;
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* Настраиваем адрес сокета */
    if(bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }
    
    /* Заполняем структуру для адреса сервера */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (servaddr.sin_port == 0)
    {
        printf("Invalid port: %s\n", argv[2]);
        close(sockfd);
        exit(1);
    }
    if(inet_aton(argv[1], &servaddr.sin_addr) == 0)
    {
        printf("Invalid IP address\n");
        close(sockfd);
        exit(1);
    }
    
    printf("Connected to chat server\n");
    printf("Waiting for server to assign username...\n");
    printf("Type your messages ('exit' to quit):\n");
    printf("You: ");
    fflush(stdout);
    
    // Не каждый раз обнуляем и добавляем, а один раз делаем нужную маски и просто копируем
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);      // Настраиваем маску, глядим на десриптор клавиатуры
    FD_SET(sockfd, &readfds); // настраиваем на сокет
    
    while(running)
    {
        readfds_copy = readfds;
        
        /* Select() для того, чтобы смотреть, где есть активность */
        if(select(sockfd + 1, &readfds_copy, NULL, NULL, NULL) < 0)
        {
            perror("select failed");
            break;
        }
        
        /* Проверяем, есть ли ввод с клавиатуры */
        if(FD_ISSET(0, &readfds_copy))
        {
            if(fgets(temp, sizeof(temp), stdin) != NULL)
            {
                char temp_copy[SECRET_SIZE];
                memcpy(temp_copy, temp, sizeof(temp_copy));
                temp[strcspn(temp, "\n")] = '\0';
                snprintf(sendline, sizeof(sendline), "%d %s", SECRET, temp);
                /* Отправляем сообщение на сервер */
                if(sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
                    perror("sendto failed");
                
                /* Проверяем команду выхода */
                if(strncmp(temp_copy, "exit", 4) == 0)
                    exit(0);
                
                printf("You: ");
                fflush(stdout);
            }
        }
        
        /* Проверяем, есть ли данные от сервера */
        if(FD_ISSET(sockfd, &readfds_copy))
        {
            int n = recvfrom(sockfd, recvline, BUFFER_SIZE-1, 0, NULL, NULL);
            if(n > 0)
            {
                recvline[n] = '\0';
                printf("\n%s", recvline);
                if (strstr(recvline, "Server: shutdown") != NULL)
                {
                    printf("\nServer is shutting down. Disconnecting...\n");
                    close(sockfd);
                    exit(0);
                }
                printf("\nYou: ");
                fflush(stdout);
            }
        }
    }
    
    char exit_msg[BUFFER_SIZE];
    snprintf(exit_msg, sizeof(exit_msg), "%d exit", SECRET);
    
    if (sendto(sockfd, exit_msg, strlen(exit_msg), 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        perror("Failed to send exit message");
    else
        printf("Sent exit notification to server\n");
    
    close(sockfd);
    printf("\nDisconnected from server.\n");
    
    return 0;
}
