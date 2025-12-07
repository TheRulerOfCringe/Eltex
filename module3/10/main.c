#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>
#include <limits.h>

#define MAX_NUMBERS 10

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
    int numbers[MAX_NUMBERS];
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
    
    int shmid;
    int size = 256;
    shmid = shmget(key, size, IPC_CREAT | 0666); // флаги: IPC_CREAT | 0666, SHM_RND тут рано

    if (shmid == -1)
    {
        perror("shmget");
        return 1;
    }
    
    // Присоединяем разделяемую память
    void *addr;
    addr = shmat(shmid, NULL, 0);
    if (addr == (void *)-1)
    {
        perror("shmat");
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
    struct shared_data *shdata = (struct shared_data*)addr;
    shdata->processed = 0;
    
    //                 --- ПРОЦЕССЫ ---
    
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        // Закрываем семафор и разделяемую память !!!!!!!!!!!!!!!!!!!!!!!!!!!
        shmctl(shmid, IPC_RMID, NULL);
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
    shmctl(shmid, IPC_RMID, NULL);
}
