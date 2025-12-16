#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>

#define BUFFER_SIZE 1000
#define MESSAGE_BUFFER_SIZE 1518
#define SECRET 2082003
#define MAX_CLIENTS 10

int running = 1;

typedef struct {
    // Храним в сетевом порядке
    uint32_t client_ip;    // iph->ip_src
    uint32_t server_ip;    // iph->ip_dst
    uint16_t client_port;  // udph->uh_sport
    uint16_t server_port;  // udph->uh_dport
    int message_count;
    bool active;
} Client;

Client clients[MAX_CLIENTS];

void signal_handler(int sig)
{
    running = 0;
}

// Функция для анализа и декодирования UDP пакета
void process_packet(const unsigned char *buffer, int size)
{
    // buffer → [IP Header][UDP Header][Data]
    struct ip *iph = (struct ip *)buffer;
    // iph->ip_hl содержит 5 слов по 4 байта каждый
    // Так что умножаем на 4
    // Всего 20
    int ip_header_len = iph->ip_hl * 4;
    struct udphdr *udph = (struct udphdr *)(buffer + ip_header_len);
    
    char source_ip[INET_ADDRSTRLEN];
    char dest_ip[INET_ADDRSTRLEN];
    
    // inet_ntop преобразует бинарный IP в строку формата "255.255.255.255"
    inet_ntop(AF_INET, &(iph->ip_src), source_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(iph->ip_dst), dest_ip, INET_ADDRSTRLEN);
    
    // ntohs преобразует 16-битное число из сетевого в хостовой порядок байт
    int source_port = ntohs(udph->uh_sport);
    int dest_port = ntohs(udph->uh_dport);
    
    // Вычисляем длину данных UDP
    int udp_header_len = sizeof(struct udphdr);
    int data_len = ntohs(udph->uh_ulen) - udp_header_len;
    
    if (data_len > 0)
    {
        const unsigned char *udp_data = buffer + ip_header_len + udp_header_len;
        
        // Создаем копию данных и добавляем '\0'
        char message[BUFFER_SIZE] = {0}, message_copy[BUFFER_SIZE] = {0};
        int copy_len = (data_len < BUFFER_SIZE - 1) ? data_len : BUFFER_SIZE - 1;
        memcpy(message, udp_data, copy_len);
        message[copy_len] = '\0';
        
        // Убираем символы новой строки для чистого вывода
        for (int i = 0; i < copy_len; i++)
            if (message[i] == '\n' || message[i] == '\r')
                message[i] = ' ';
                
        memcpy(message_copy, message, sizeof(message));
        
        char parse_buf[BUFFER_SIZE];
        memcpy(parse_buf, message_copy, sizeof(parse_buf));

        // Пошла обработка клиента
        char *token = strtok(parse_buf, " ");  // Первая часть
        
        //printf("\n\n%d\n\n", atoi(token));
        
        // Если наш же пикет, то мы игнорируем (хотя +1 пойманный пакет всё равно засчитывается)
        bool is_our_packet = false;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].server_ip == iph->ip_src.s_addr && clients[i].server_port == udph->uh_sport)
            {
                is_our_packet = true;
                break;
            }
        }

        if (is_our_packet)
        {
            printf("Ignoring packet from us (port %d)\n", ntohs(udph->uh_sport));
            return;
        }

        if (token)
        {
            if (atoi(token) == SECRET)
            {
                int free_client = -1;
                int our_client = -1;
                token = strtok(NULL, "");  // Остаток строки
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (!clients[i].active)
                    {
                        if (free_client == -1)
                        {
                            free_client = i;
                        }
                    }
                    else
                    {
                        if (iph->ip_src.s_addr == clients[i].client_ip && udph->uh_sport == clients[i].client_port)
                            {
                                our_client = i;
                            }
                    }
                }
                if (our_client == -1 && free_client != -1)
                {
                    // our_client == -1 -> не нашли клиента
                    // Добавляем нового
                    clients[free_client].client_ip = iph->ip_src.s_addr;
                    clients[free_client].server_ip = iph->ip_dst.s_addr;
                    clients[free_client].client_port = udph->uh_sport;
                    clients[free_client].server_port = udph->uh_dport;
                    clients[free_client].active = true;
                    clients[free_client].message_count = 0;
                    our_client = free_client;
                }
                else if (our_client == -1 && free_client == -1)
                {
                    // Как реагироват на переполнение?..
                    ;
                    return; // Пока просто без реакции
                }
                
                if (strcmp(token, "exit") == 0)
                {
                    if (our_client != -1)
                    {
                        clients[our_client].active = false;
                        clients[our_client].client_ip = 0;
                        clients[our_client].client_port = 0;
                        //clients[our_client].server_ip = 0;
                        //clients[our_client].server_port = 0;
                        clients[our_client].message_count = 0;
                        
                        printf("Client %s:%d disconnected\n", inet_ntoa((struct in_addr){iph->ip_src.s_addr}), ntohs(udph->uh_sport));
                    }
                    return;
                }
                else
                {
                    // Готовим данные
                    
                    char sendline[BUFFER_SIZE];
                    clients[our_client].message_count++;
                    token[strcspn(token, "\0")] = ' ';
                    snprintf(sendline, sizeof(sendline), "%s%d", token, clients[our_client].message_count);
                    // printf("sendline: %s\nour_client: %d\nfree_client: %d\n", sendline, our_client, free_client);
                    
                    // Отправляем ответ с message_count
                    
                    int sock_responce;
                    struct sockaddr_in servaddr, cliaddr;
                    
                    if((sock_responce = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
                    {
                        perror("Socket creation failed");
                        exit(1);
                    }
                                                  
                    /* Заполняем структуру для адреса клиента */
                    bzero(&cliaddr, sizeof(cliaddr));
                    cliaddr.sin_family = AF_INET;
                    cliaddr.sin_port = clients[our_client].client_port;
                    cliaddr.sin_addr.s_addr = clients[our_client].client_ip;
                    
                    /* Заполняем структуру для адреса сервера */
                    bzero(&servaddr, sizeof(servaddr));
                    servaddr.sin_family = AF_INET;
                    servaddr.sin_port = clients[our_client].server_port;
                    servaddr.sin_addr.s_addr = clients[our_client].server_ip;
                                    
                    //check_port_status(servaddr.sin_port);
                    
                    /* Настраиваем адрес сокета */
                    if(bind(sock_responce, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
                    {
                        perror("Bind failed");
                        close(sock_responce);
                        exit(1);
                    }
                    
                    if(sendto(sock_responce, sendline, strlen(sendline), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
                        perror("sendto failed");
                    
                    close(sock_responce);
                    
                    printf("Packet info:\n");
                    printf("From: %s:%d\n", source_ip, source_port);
                    printf("To: %s:%d\n", dest_ip, dest_port);
                    printf("Data %d bytes: \"%s\"\n\n", data_len, message_copy);
                }
            }
        }
    }
}

int main()
{
    signal(SIGINT, signal_handler);
    
    if (getuid() != 0)
    {
        printf("Warning: Run like root!\n");
        exit(1);
    }
    
    int raw_socket;
    unsigned char buffer[MESSAGE_BUFFER_SIZE];
 
    // Создаем RAW сокет для перехвата UDP пакетов
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_socket < 0)
    {
        perror("socket");
        exit(1);
    }
    
    printf("Server started using AF_INET. Listening for UDP packets on all ports...\n");
    
    int packet_count = 0;
    
    struct timeval tv;
    tv.tv_sec = 1;   // 1 секунда таймаута для корректного выхода по SIGINT
    tv.tv_usec = 0;
    setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    while (running)
    {
        int data_size = recv(raw_socket, buffer, MESSAGE_BUFFER_SIZE, 0);
        printf("%d\n", packet_count);

        if (data_size > 0)
        {
            // Анализируем пакеты достаточного размера
            // struct ip (IP заголовок) = 20 байт
            // struct udphdr (UDP заголовок) = 8 байт
            // Минимум 28 байт для валидного UDP/IP пакета
            if (data_size >= sizeof(struct ip) + sizeof(struct udphdr))
            {
                process_packet(buffer, data_size);
                packet_count++;
            }
        } 
        else if (data_size < 0)
        {
            // EAGAIN и EWOULDBLOCK - "Попробуй позже"
            // EINTR - системный вызов был прерван сигналом
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                // Таймаут или прерывание - продолжаем цикл
                continue;
            }
            else
            {
                perror("recv");
                break;
            }
        }
        
        // Пауза
        sleep(1);
    }
    
    char shutdown_msg[] = "Server: shutdown";
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].active)
        {
            printf("Notifying client %d: %s:%d\n", i, inet_ntoa((struct in_addr){clients[i].client_ip}), ntohs(clients[i].client_port));
            fflush(stdout);
            
            int sock_shotdown;
            struct sockaddr_in servaddr, cliaddr;
            
            if ((sock_shotdown = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
            {
                perror("Socket creation failed");
            }
                            
            /* Заполняем структуру для адреса клиента */
            bzero(&cliaddr, sizeof(cliaddr));
            cliaddr.sin_family = AF_INET;
            cliaddr.sin_port = clients[i].client_port;
            cliaddr.sin_addr.s_addr = clients[i].client_ip;
            
            /* Заполняем структуру для адреса сервера */
            bzero(&servaddr, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = clients[i].server_port;
            servaddr.sin_addr.s_addr = clients[i].server_ip;
            
            /* Настраиваем адрес сокета */
            if (bind(sock_shotdown, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
            {
                perror("Bind failed");
                close(sock_shotdown);
            }
            
            if(sendto(sock_shotdown, shutdown_msg, strlen(shutdown_msg), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
                perror("sendto failed");
            
            close(sock_shotdown);
            
        }
    }
    
    printf("\nServer shutdown.\n");
    
    close(raw_socket);
    return 0;
}
