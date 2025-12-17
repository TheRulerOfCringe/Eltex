ЗАДАНИЕ: epoll + Unix Domain Socket

1. epoll

    1.1 Подключаем
    
    <sys/epoll.h>

    1.2 Ициниализируеум события
    
    struct epoll_event ev, events[MAX_EVENTS];

    // ПРОКОММЕНТРИКУЙ ПОЧЕМУ ИХ ДВА

    1.3 Создаём epoll + проверка, что всё успешно
    
    int epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    
    1.4 Добавляем событие
    
        1.4.1 Событие "ввод с квалиатуры" у сервера
        
        ev.events = EPOLLIN;
        ev.data.fd = STDIN_FILENO;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1)
        {
            perror("epoll_ctl: stdin");
            exit(EXIT_FAILURE);
        }
        
        1.4.2 ДОБАВЬ, КАК СДЕЛАЕШЬ UNIX DOMAIN SOCKETS
        
        s
        
    1.5 Обнуляем ev между событиями
    
    memset(&ev, 0, sizeof(ev));
    
    1.6 Ожидаем событие
    
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    // nfds - количество дескрипторов, на которых произошло событие
    // epollfd - дескрипторы, за которыми мы наблюдаем, создался в 1.3
    // events - массив событий, инициализировали в 1.2
    // MAX_EVENTS - максимальное количество событий массива events
    // -1 - будем ждать бесконечно. А можем количество миллискунд ввести
    
    1.7 Проверяем, что за событие, предпринимаем нужные действия
    
    if (events[n].data.fd == STDIN_FILENO)
    {
        ...
    }
    else
    {
        ...
    }
    
    1.8 Удаляем событие
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, drivers[num_to_kill].sv, NULL) == -1)
    {
        perror("epoll_ctl_delete: sv");
    }

    // NULL последним аргументом для совместимости, ни на что не влияет

2. Unix Domain Socket (UDS)

    2.1 Подключаем библиотеку
    
    #include <sys/socket.h>
    
    2.2 Создаём массив из двух дескрипторов
    
    int sv[2];
    
    2.3 Создаём сокет
    
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    // AF_UNIX - флаг, что создаём два Unix Domain сокета
    // SOCK_STREAM - двухсторонний поток байтов
    // 0 - для AF_UNIX тут всегда 0
    // sv - массив дескрипторов
    
    2.4 Проверяем успешность создания
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1)
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }
    
    2.5 Закрываем ненужные концы
    
    // У родительского процесса
    close(sv[1]);
    
    // У дочернего процесса
    close(sv[0]);
    
    2.6 Отправка сообщений
    
    2.7 Принятие сообщения

3 timerfd

    3.1 Подключаем библиотеки
    
    #include <sys/timerfd.h>
    #include <time.h>
    
    3.2 Создаём дескриптор
    
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    // CLOCK_MONOTONIC нужно, чтобы часы не менялись при корректировке системного времени
    // TFD_NONBLOCK нужно, чтобы не блокировать, а то какой смысл в epoll
    if (timer_fd == -1)
    {
        perror("timerfd_create");
        exit(EXIT_FAILURE);
    }
    
    3.3 Создаём структуру спецификации таймера
    
    struct itimerspec timer_spec;
    memset(&timer_spec, 0, sizeof(timer_spec));

    timer_spec.it_value.tv_sec = 200; // секунд
    timer_spec.it_value.tv_nsec = 0; // наносекунд

    timer_spec.it_interval.tv_sec = 0; // повтор через 0 секунд -> не повторять
    timer_spec.it_interval.tv_nsec = 0;
    
    3.4 Настраиваем timer_fd этой структурой
    
    if (timerfd_settime(timer_fd, 0, &timer_spec, NULL) == -1)
    {
        perror("timerfd_settime");
        close(timer_fd);
        exit(EXIT_FAILURE);
    }
    // Можно получать старое значение таймера при перенастройке там, где NULL



token сразу в int без проверок на корректность

Не передавайтте в таймер отрицательные числа, пж пж пж пж пж

Как будто нужен отдельный BUFFER_SIZE для сообщений-команд, а то 1024 жирно.

ЧТО ПО FREE_DRIVER?
Если удалил, то надо как-то вернуть
Хотя в задании нет удаления -> нет таких проблем
В общем, надо подумать/обговорить моментик
    
