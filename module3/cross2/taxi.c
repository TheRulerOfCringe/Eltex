#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <time.h>

#define MAX_EVENTS 10
#define MAX_DRIVERS 10
#define BUFFER_SIZE 1024

bool running = true;

void signal_handler(int sig)
{
    running = false;
}

typedef struct
{
    pid_t pid;
    int sv;
    bool deleted;
} driver;

driver drivers[MAX_DRIVERS];
int free_driver = 0;

int find_num_by_pid(int pid)
{
    for (int i = 0; i < MAX_DRIVERS; i++)
        if (drivers[i].pid == pid)
            return i;
    return -1; // -1 вернёт, если не нашёл
}

int create_driver()
{
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1)
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }
    
	pid_t pid;
	pid = fork();
	if (pid == 0) // Дочерний процесс driver
	{
	    printf("Дочерний процесс driver начался с pid = %d. И это логично.\n", pid);
	    close(sv[0]);
	    int num = -1;
	    bool busy = false;
	    
	    // epoll дочернего процесса
	    struct epoll_event ev, events[MAX_EVENTS];
	    
        int epollfd = epoll_create1(0);
        if (epollfd == -1)
        {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }
	    
        ev.events = EPOLLIN;
        ev.data.fd = sv[1];
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sv[1], &ev) == -1)
        {
            perror("epoll_ctl: sv[1]");
            exit(EXIT_FAILURE);
        }
        
        int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        if (timer_fd == -1)
        {
            perror("timerfd_create");
            exit(EXIT_FAILURE);
        }
        
        bool driving = true;
        
        while(driving)
        {
            int nfds;
            nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);

            if (nfds == -1)
            {
                if (errno == EINTR)
                {
                    // Сигнал SIGINT прервал epoll_wait
                    printf("\nWe are INTerrupted\n");
                    break;
                }
                else
                {
                    perror("epoll_wait");
                    exit(EXIT_FAILURE);
                }
            }
            
            for (int n = 0; n < nfds; n++)
            {
                if (events[n].data.fd == sv[1])
                {
                    char cmd[BUFFER_SIZE];
                    ssize_t bytes_cmd = read(sv[1], cmd, sizeof(cmd) - 1);
                    cmd[bytes_cmd] = '\0';
                    printf("Получено от родителя: '%s'\n", cmd);
                    printf("bytes = %ld.\n", bytes_cmd);
                    
                    if (bytes_cmd > 0)
                    {
                        char *token = strtok(cmd, " ");
                        if (strcmp(token, "n") == 0)
                        {
                            token = strtok(NULL, " ");
                            num = atoi(token);
                            printf("I am driver number %d now!\n", num);
                        }
                        else if (strcmp(token, "t") == 0)
                        {
                            // Поднимает таймер
                            token = strtok(NULL, " ");
                            int time = atoi(token);
                            
                            // Теперь не тут инициализируем
                            
                            struct itimerspec timer_spec;
                            memset(&timer_spec, 0, sizeof(timer_spec));

                            timer_spec.it_value.tv_sec = time;
                            timer_spec.it_value.tv_nsec = 0;

                            timer_spec.it_interval.tv_sec = 0;
                            timer_spec.it_interval.tv_nsec = 0;

                            if (timerfd_settime(timer_fd, 0, &timer_spec, NULL) == -1)
                            {
                                perror("timerfd_settime");
                                close(timer_fd);
                                exit(EXIT_FAILURE);
                            }
                            
                            memset(&ev, 0, sizeof(ev));
                            ev.events = EPOLLIN;
                            ev.data.fd = timer_fd;
                            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, timer_fd, &ev) == -1)
                            {
                                perror("epoll_ctl: timer_fd");
                                exit(EXIT_FAILURE);
                            }
                            busy = true;
                        }
                        else if (strcmp(token, "s") == 0)
                        {
                            //char cmp_s_responce[BUFFER_SIZE];
                            //sprintf(cmp_s_responce, "s %d", busy);
                            char cmp_s_responce[BUFFER_SIZE];
                            sprintf(cmp_s_responce, "s %d", busy);
                            write(sv[1], cmp_s_responce, strlen(cmp_s_responce));
                        }
                        else
                        {
                            printf("What the heck, who am i?\n");
                        }
                        
                    }
                    else if (bytes_cmd == 0)
                    {
                        driving = false;
                        drivers[num].deleted = true;
                        drivers[num].pid = -1;
                    }
                    else
                    {
                        perror("read");
                    }
                }
                else if (events[n].data.fd == timer_fd)
                {
                    busy = false;
                    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, timer_fd, NULL) == -1)
                    {
                        perror("epoll_ctl_delete: timer_fd");
                    }
                }
            }
            
        }
        printf("Driver is dead.\n");
        close(sv[1]);
	    exit(0);
	}
	else if (pid > 0)
	{
	    printf("Родительский процесс знает, что у дочернего процесса pid = %d.\n", pid);
	    close(sv[1]);
	    drivers[free_driver].pid = pid;
	    drivers[free_driver].sv = sv[0];
	    
	    
	    
	    return 0;
	}
	else
	{
	    perror("fork");
	    return -1;
	}
}

int main()
{
    signal(SIGINT, signal_handler);
    int pid = getpid();
    printf("Jandex Taxi initiated with pid = %d.\n", pid);
    
    // Инициализация epoll
    struct epoll_event ev, events[MAX_EVENTS]; // Понадобитсья ли этот массив, увидим
    int nfds;
    
    int epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    printf("epollfd = %d\n",epollfd);
    
    // Чтение стандартного ввода в epoll
    ev.events = EPOLLIN; // Флаг на чтение
    ev.data.fd = STDIN_FILENO;
    
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1)
    {
        perror("epoll_ctl: stdin");
        exit(EXIT_FAILURE);
    }

    char buf[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // Чтобы -1 в pid было маркером отсутствия driver'а в get_drivers
    for (int i = 0; i < MAX_DRIVERS; i++)
        drivers[i].pid = -1;
    
    printf("Type \"help\" for list commands!\n");
    while (running)
    {
        sleep(1); // Чтобы сообщения всяких детей не накладывались.
        memset(buf, '\0', sizeof(buf));
        printf(">");
        fflush(stdout);
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            if (errno == EINTR)
            {
                // Сигнал SIGINT прервал epoll_wait
                printf("\nWe are INTerrupted\n");
                break;
            }
            else
            {
                perror("epoll_wait");
                exit(EXIT_FAILURE);
            }
        }
        
        for (int n = 0; n < nfds; n++)
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
                printf("Text: \"%s\"\n", buf);
                
                char *token = strtok(buf, " ");
                
                // Выбор команд начнётся тут
                if (strcmp(token, "help") == 0)
                {
                    printf(" - \"create_driver\"                to create new driver to do tasks.\n");
                    printf(" - \"send_task <pid> <task_timer>\" to send task for <task_timer> seconds to driver with <pid>.\n");
                    printf(" - \"get_status <pid>\"             to get status of driver with <pid>.\n");
                    printf(" - \"get_drivers\"                  to get statuses and pids of all drivers exist.\n");
                    printf(" - \"help\"                         to print this message once again. For fun, i guess.\n");
                    printf(" - \"exit\"                         to kill all drivers and this program respectively.\n");
                    printf(" - \"kill <pid>\"                   to kill driver with <pid>.\n");
                }
                else if (strcmp(token, "create_driver") == 0)
                {
                    printf("Driver zavodit mashinu.\n");
                    int result = create_driver();
                    
                    if (result < 0)
                    {
                        printf("Driver zagloh.\n");
                    }
                    else
                    {
                        memset(&ev, 0, sizeof(ev));
                        ev.events = EPOLLIN;
                        ev.data.fd = drivers[free_driver].sv;
                        
                        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, drivers[free_driver].sv, &ev) == -1)
                        {
                            perror("epoll_ctl: sv");
                            exit(EXIT_FAILURE);
                        }
                        char cmp_n[BUFFER_SIZE];
                        sprintf(cmp_n, "n %d", free_driver);
                        write(drivers[free_driver].sv, cmp_n, strlen(cmp_n));
                        free_driver++;
                    }
                }
                else if (strcmp(token, "send_task") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        printf("Key needed. Usage: send_task <pid> <task_timer>.\n");
                    }
                    else
                    {
                        int pid_to_send_task = atoi(token);
                        token = strtok(NULL, " ");
                        if (token == NULL)
                        {
                            printf("Key needed. Usage: send_task <pid> <task_timer>.\n");
                        }
                        else
                        {
                            int task_timer = atoi(token);
                            token = strtok(NULL, " ");
                            if (token != NULL)
                            {
                                printf("Extra keys in command line.\n");
                            }
                            else
                            {
                                int num_to_send_task = find_num_by_pid(pid_to_send_task);
                                if (num_to_send_task < 0)
                                {
                                    printf("There is no driver with such pid. No one was given a task.\n");
                                }
                                else
                                {
                                    char cmp_t[BUFFER_SIZE];
                                    sprintf(cmp_t, "t %d", task_timer);
                                    write(drivers[num_to_send_task].sv, cmp_t, strlen(cmp_t));
                                }
                            }
                        }
                    }
                }
                else if (strcmp(token, "get_status") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        printf("Key needed. Usage: get_status <pid>.\n");
                    }
                    else
                    {
                        int pid_to_get_status = atoi(token);
                        token = strtok(NULL, " ");
                        if (token != NULL)
                        {
                            printf("Extra keys in command line.\n");
                        }
                        else
                        {
                            int num_to_get_status = find_num_by_pid(pid_to_get_status);
                            if (num_to_get_status < 0)
                            {
                                printf("There is no driver with such pid. No one was asked about status.\n");
                            }
                            else
                            {
                                write(drivers[num_to_get_status].sv, "s", strlen("s"));
                                /*
                                bytes_read = read(drivers[num_to_get_status].sv, buf, BUFFER_SIZE - 1);
                                if (bytes_read < 0)
                                {
                                    perror("read");
                                    exit(EXIT_FAILURE);
                                }
                                buf[strcspn(buf, "\n")] = '\0';
                                bool busy = atoi(buf[0]);
                                printf("Driver with pid %d is ", num_to_get_status);
                                if (busy)
                                {
                                    printf("busy at the moment.\n");
                                }
                                else
                                {
                                    printf("free at the moment.\n");
                                }
                                */
                            }
                        }
                    }
                }
                else if (strcmp(token, "get_drivers") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        for (int i = 0; i < MAX_DRIVERS; i++)
                            if (drivers[i].pid > 0)
                                {
                                    printf("Driver number %d has pid %d.\n", i, drivers[i].pid);
                                    // Можно ещё сразу busy добавить
                                }
                    }
                    else
                    {
                        printf("Extra keys in command line.\n");
                    }
                }
                else if (strcmp(token, "kill") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        printf("Key needed. Usage: kill <pid>.\n");
                    }
                    else
                    {
                        int pid_to_kill = atoi(token);
                        token = strtok(NULL, " ");
                        if (token != NULL)
                        {
                            printf("Extra keys in command line.\n");
                        }
                        else
                        {
                            int num_to_kill = find_num_by_pid(pid_to_kill);
                            if (num_to_kill < 0)
                            {
                                printf("There is no driver with such pid. No one was killed.\n");
                            }
                            else
                            {
                                if (epoll_ctl(epollfd, EPOLL_CTL_DEL, drivers[num_to_kill].sv, NULL) == -1)
                                {
                                    perror("epoll_ctl_delete: sv");
                                }
                                close(drivers[num_to_kill].sv);
                                drivers[num_to_kill].deleted = true;
                            }
                        }
                        
                    }
                }
                else if (strcmp(token, "exit") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        running = false;
                    }
                    else
                    {
                        printf("Extra keys in command line.\n");
                    }
                }
                else
                {
                    printf("Unknown command \"%s\".\n", token);
                }
            }
            else
            {
                bytes_read = read(events[n].data.fd, buf, BUFFER_SIZE - 1);
                if (bytes_read < 0)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                
                buf[strcspn(buf, "\n")] = '\0';
                printf("Text: \"%s\"\n", buf);
                
                char *token = strtok(buf, " ");
                
                // Выбор команд начнётся тут
                if (strcmp(token, "s") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token != NULL)
                    {
                        bool busy = atoi(token);
                        if (busy)
                        {
                            printf("Driver is busy at the moment.\n");
                        }
                        else
                        {
                            printf("Driver is free at the moment.\n");
                        }
                    }
                    else
                    {
                        printf("Something gone wrong.\n");
                    }
                }
                else
                {
                    printf("Something gone wrong.\n");
                }
            }
            //else if (events[n].data.fd == А_КАКОЙ_ЭТО_DRIVER)
            //{
            //
            //}
            //else
            //{
                //printf("%d: %d.\n", n, events[n].data.fd);
                //printf("What the hell, who am i?\n");
            //}
        }
        
        //sleep(1);
    }
    
    for (int i = 0; i < MAX_DRIVERS; i++)
        if (drivers[i].pid > 0)
            close(drivers[i].sv);
    
    sleep(1);
    printf("\nJandex Taxi ruined.\n");
}
