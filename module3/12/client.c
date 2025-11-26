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

#define SERVER_PORT 12345
#define BUFFER_SIZE 1000

int running = 1;
void signal_handler(int sig)
{
    running = 0;
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    fd_set readfds;
    char sendline[BUFFER_SIZE], recvline[BUFFER_SIZE];
    
    if(argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
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
    cliaddr.sin_port = htons(0);
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
    servaddr.sin_port = htons(SERVER_PORT);
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
    
    while(running)
    {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);      // Настраиваем маску, глядим на десриптор клавиатуры
        FD_SET(sockfd, &readfds); // настраиваем на сокет
        
        /* Select() для того, чтобы смотреть, где есть активность */
        if(select(sockfd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select failed");
            break;
        }
        
        /* Проверяем, есть ли ввод с клавиатуры */
        if(FD_ISSET(0, &readfds))
        {
            if(fgets(sendline, BUFFER_SIZE, stdin) != NULL)
            {
                /* Отправляем сообщение на сервер */
                if(sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
                    perror("sendto failed");
                
                /* Проверяем команду выхода */
                if(strncmp(sendline, "exit", 4) == 0)
                    break;
                
                printf("You: ");
                fflush(stdout);
            }
        }
        
        /* Проверяем, есть ли данные от сервера */
        if(FD_ISSET(sockfd, &readfds))
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
                printf("You: ");
                fflush(stdout);
            }
        }
    }
    
    close(sockfd);
    printf("\nDisconnected from chat.\n");
    
    return 0;
}
