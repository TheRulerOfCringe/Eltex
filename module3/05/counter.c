#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

volatile sig_atomic_t counter = 0;
volatile sig_atomic_t sigint_count = 0;
int fd = -1;

void signal_handler(int sig)
{
    char buffer[256];
    int len;
    
    if (sig == SIGINT)
    {
        len = snprintf(buffer, sizeof(buffer), "Got SIGINT!\n");
        write(fd, buffer, len);
        sigint_count++;
        if (sigint_count >= 3)
        {
            len = snprintf(buffer, sizeof(buffer), "Got 3 SIGINT!!! Quiting.\n");
            write(fd, buffer, len);
            close(fd);
            exit(0);
        }
    }
    else if (sig == SIGQUIT)
    {
        len = snprintf(buffer, sizeof(buffer), "Got SIGQUIT!\n");
        write(fd, buffer, len);
        close(fd);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    // Проверяем аргументы командной строки
    if (argc != 2)
    {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }
    
    // Открываем файл для записи через файловый дескриптор
    fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Ошибка открытия файла");
        return 1;
    }
    
    // Устанавливаем обработчики сигналов
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Ошибка установки обработчика SIGINT");
        close(fd);
        return 1;
    }
    
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
    {
        perror("Ошибка установки обработчика SIGQUIT");
        close(fd);
        return 1;
    }
    
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "Программа запущена. PID: %d\n", getpid());
    write(fd, buffer, len);
    
    while (1)
    {
        counter++;
        len = snprintf(buffer, sizeof(buffer), "%d\n", counter);
        write(fd, buffer, len);
        sleep(1);
    }
    
    close(fd);
    return 0;
}
