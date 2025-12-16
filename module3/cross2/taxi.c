#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

bool running = true;

void signal_handler(int sig)
{
    running = false;
}

int main()
{
    signal(SIGINT, signal_handler);
    int pid = getpid();
    printf("Jandex Taxi initiated with pid = %d.\n", pid);
    
    struct epoll_event ev, events[MAX_EVENTS]; // Понадобитсья ли этот массив, увидим
    int nfds;
    
    int epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    printf("epollfd = %d\n",epollfd);
    
    // Чтение стандартного ввода
    ev.events = EPOLLIN; // Флаг на чтение
    ev.data.fd = STDIN_FILENO;
    
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1)
    {
        perror("epoll_ctl: stdin");
        exit(EXIT_FAILURE);
    }


    char buf[BUFFER_SIZE];
    ssize_t bytes_read;
    
    printf("Lets go  to while!\n");
    while (running)
    {
        memset(buf, '\0', sizeof(buf));
        printf("\n>");
        fflush(stdout);
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            if (errno == EINTR)
            {
                // Сигнал прервал epoll_wait
                printf("\nWe are INTerrupted\n");
                break;
            }
            else
            {
                perror("epoll_wait");
                exit(EXIT_FAILURE);
            }
        }
        
        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == STDIN_FILENO)
            {
                bytes_read = read(STDIN_FILENO, buf, BUFFER_SIZE - 1);
                if (bytes_read < 0)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                //printf("%d: %d.\n", n, events[n].data.fd);
                buf[strcspn(buf, "\n")] = '\0';
                printf("Text: \"%s\"", buf);
            }
            else
            {
                printf("%d: %d.\n", n, events[n].data.fd);
                printf("What the hell, who am i?\n");
            }
        }
        
        //sleep(1);
    }
    
    
    
    
    
    printf("\nJandex Taxi ruined.\n");
}
