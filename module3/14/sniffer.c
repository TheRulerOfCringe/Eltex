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

#define PORT 12345
#define BUFFER_SIZE 1024
#define DUMP_FILENAME "packet_dump.bin"
#define DECODED_FILENAME "decoded_messages.txt"

int running = 1;

void signal_handler(int sig)
{
    running = 0;
}

// Функция для анализа и декодирования UDP пакета
void decode_udp_packet(const unsigned char *buffer, int size, FILE *dump_file, FILE *decoded_file)
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
    
    if (data_len > 0 && dest_port == PORT)
    {
        const unsigned char *udp_data = buffer + ip_header_len + udp_header_len;
        
        // Записываем полный дамп пакета
        if (dump_file != NULL)
        {
            fwrite(buffer, 1, size, dump_file);
            fflush(dump_file);
        }
        
        // Создаем копию данных и добавляем '\0'
        char message[BUFFER_SIZE];
        int copy_len = (data_len < BUFFER_SIZE - 1) ? data_len : BUFFER_SIZE - 1;
        memcpy(message, udp_data, copy_len);
        message[copy_len] = '\0';
        
        // Убираем символы новой строки для чистого вывода
        for (int i = 0; i < copy_len; i++)
            if (message[i] == '\n' || message[i] == '\r')
                message[i] = ' ';
        
        printf("Intercepted UDP packet:\n");
        printf("From: %s:%d\n", source_ip, source_port);
        printf("To: %s:%d\n", dest_ip, dest_port);
        printf("Data %d bytes: \"%s\"\n\n", data_len, message);
        
        // Записываем в файл расшифрованных сообщений
        if (decoded_file != NULL)
        {
            fprintf(decoded_file, "Intercepted UDP packet:\n");
            fprintf(decoded_file, "Source: %s:%d\n", source_ip, source_port);
            fprintf(decoded_file, "Destination: %s:%d\n", dest_ip, dest_port);
            fprintf(decoded_file, "Length: %d bytes\n", data_len);
            fprintf(decoded_file, "Message: \"%s\"\n\n", message);
            fflush(decoded_file);
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
    unsigned char buffer[BUFFER_SIZE];
    
    FILE *dump_file = fopen(DUMP_FILENAME, "wb");
    
    if (dump_file == NULL)
        perror("fopen dump file");
    
    FILE *decoded_file = fopen(DECODED_FILENAME, "w");
    
    if (decoded_file == NULL)
        perror("fopen decoded file");
 
    // Создаем RAW сокет для перехвата UDP пакетов
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_socket < 0)
    {
        perror("socket");
        printf("Note: RAW sockets require root privileges!\n");
        if (dump_file)
            fclose(dump_file);
        if (decoded_file)
            fclose(decoded_file);
        exit(1);
    }
    
    printf("Sniffer started using AF_INET. Listening for UDP packets on port %d...\n", PORT);
    printf("Dump file: %s\n", DUMP_FILENAME);
    printf("Decoded messages: %s\n\n", DECODED_FILENAME);
    
    if (decoded_file != NULL)
    {
        fprintf(decoded_file, "Sniffer turn on\n");
        fprintf(decoded_file, "Port: %d\n\n", PORT);
        fflush(decoded_file);
    }
    
    int packet_count = 0;
    
    struct timeval tv;
    tv.tv_sec = 1;   // 1 секунда таймаута для корректного выхода по SIGINT
    tv.tv_usec = 0;
    setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    while (running)
    {
        int data_size = recv(raw_socket, buffer, BUFFER_SIZE, 0);

        if (data_size > 0)
        {
            // Анализируем пакеты достаточного размера
            // struct ip (IP заголовок) = 20 байт
            // struct udphdr (UDP заголовок) = 8 байт
            // Минимум 28 байт для валидного UDP/IP пакета
            if (data_size >= sizeof(struct ip) + sizeof(struct udphdr))
            {
                decode_udp_packet(buffer, data_size, dump_file, decoded_file);
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
    
    printf("\nSniffer shutdown.\n");
    
    if (decoded_file != NULL)
    {
        fprintf(decoded_file, "\nSniffer turn off\n");
        fclose(decoded_file);
    }
    
    if (dump_file != NULL)
        fclose(dump_file);
    
    close(raw_socket);
    return 0;
}
