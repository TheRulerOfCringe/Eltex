#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>
#include <limits.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

int running = 1;
int sem_id = -1;
union semun
{
      int val;                  /* value for SETVAL */
      struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
      unsigned short *array;    /* array for GETALL, SETALL */
                                /* Linux specific part: */
      struct seminfo *__buf;    /* buffer for IPC_INFO */
};

struct sembuf lock = {0, -1, 0};
// 0 - работаем с семафором #0 в наборе
// -1 - хотим уменьшить значение семафора на 1
// 0 - флаги (0 = ждать, если операция не может быть выполнена)

struct sembuf unlock[2] = {{0, 0, 0}, {0, 1, 0}};

struct shared_data
{
    int numbers[10];
    int count;
    int min;
    int max;
    int processed;
    // 0 - дочерний процесс всё сделал, ждём родительский (ну или только инициализировали)
    // 1 - родительский всё сделал, ждёт дочерний
};

void signal_handler(int sig)
{
    running = 0;
}

int main()
{
    signal(SIGINT, signal_handler);
    
    pid_t pid;
    
    // Тут создадим семафор и разделяемую память
    key_t key;
    if ((key = ftok(".", 'A')) == -1)
    {
        perror("ftok");
        return 1;
    }
    
    //                 --- РАЗДЕЛЯЕМАЯ ПАМЯТЬ ---
    
    const char *shm_name = "/my_shm";
    int shm_fd;
    struct shared_data *shdata;
    size_t shm_size = sizeof(struct shared_data);
    
    // Создаем или открываем разделённую память
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return 1;
    }
    
    // Устанавливаем размер разделяемой памяти
    if (ftruncate(shm_fd, shm_size) == -1)
    {
        perror("ftruncate");
        return 1;
    }
    
    // Отображаем разделяемую память в адресное пространство процесса
    shdata = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shdata == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }
    
    //                 --- СЕМАФОР ---
    
    int sem_id;
    // Создаем семафор
    if ((sem_id = semget(key, 1, IPC_CREAT | 0666)) == -1)
    {
        perror("semget");
        return 1;
    }

    union semun arg; // Создаем переменную типа union semun
    arg.val = 1;     // Заполняем поле val значением 1
    if (semctl(sem_id, 0, SETVAL, arg) == -1) // Устанавливаем значение семафора
    {
        perror("semctl");
        return 1;
    }
    // Проверка, что создался сегмент разделяемой памяти

    
    srand(time(NULL));
    shdata->processed = 0;
    
    //                 --- ПРОЦЕССЫ ---
    
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        // Закрываем семафор и разделяемую память !!!!!!!!!!!!!!!!!!!!!!!!!!!
        shm_unlink(shm_name);
        return 1;
    }
    
    
    
    if (pid > 0)
    {
        // Родительский процесс
        printf("Hi, im parent!\n");
        
        int obrabotannih = 0;
        
        // Один раз инициализируем до цикла, чтобы min max не ломался
        
        // Захватываем семафор
        if (semop(sem_id, &lock, 1) == -1)
            perror("semop lock");
        
        shdata->count = 5 + rand() % 6; // 5-10
        for (int i = 0; i < shdata->count; i++)
            shdata->numbers[i] = rand() % 100; // 0-99
        shdata->processed = 1;
        
        // Освобождаем семафор
        if (semop(sem_id, unlock, 2) == -1)
            perror("semop unlock");
        
        while(running)
        {
            sleep(1);
            
            if (semop(sem_id, &lock, 1) == -1)
            {
                perror("semop lock");
                break;
            }
            
            if (shdata->processed == 0) // МОЖЕТ, МОЖНО ПРОВЕРЯТЬ БЕЗ БЛОКИРОВКИ?
            {
                printf("Parent got min = %d, max = %d.\n", shdata->min, shdata->max);
                shdata->count = 5 + rand() % 6; // 5-10
                for (int i = 0; i < shdata->count; i++)
                {
                    shdata->numbers[i] = rand() % 100; // 0-99
                    //printf("%d ",shdata->numbers[i]);
                }
                printf("\n");
                shdata->processed = 1;
                obrabotannih++;
            }
            
            if (semop(sem_id, unlock, 2) == -1)
            {
                perror("semop unlock");
                break;
            }
        }
        printf("\nThere were %d obrabotannih naborov.\n", obrabotannih);
    }
    else
    {
        // Дочерний процесс
        printf("Hi, im child!\n");
        while(running)
        {
            sleep(1);
            
            if (semop(sem_id, &lock, 1) == -1)
            {
                perror("semop lock");
                break;
            }
            
            if (shdata->processed == 1)
            {
                int min = INT_MAX;
                int max = INT_MIN;
                printf("Child got %d numbers.\n", shdata->count);
                for (int i = 0; i < shdata->count; i++)
                {
                    if (shdata->numbers[i] < min)
                        min = shdata->numbers[i];
                    if (shdata->numbers[i] > max)
                        max = shdata->numbers[i];
                }
                shdata->min = min;
                shdata->max = max;
                shdata->processed = 0;
            }
            
            if (semop(sem_id, unlock, 2) == -1)
            {
                perror("semop unlock");
                break;
            }
        }
        exit(0);
    }
    
    // Удаляем семафор
    semctl(sem_id, 0, IPC_RMID);
    // Удаляем сегмент разделяемой памяти
    shm_unlink(shm_name);
}
