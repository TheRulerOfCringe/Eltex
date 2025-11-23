#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>

// ps aux | grep consumer
// kill -STOP ID
// kill -CONT ID

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

void signal_handler(int sig)
{
    printf("\nConsumer: Received signal %d, shutting down...\n", sig);
    
    // Освобождаем семафор если он был захвачен (а надо ли?)
    if (sem_id != -1)
    {
        printf("Consumer: Releasing semaphore...\n");
        semop(sem_id, unlock, 2);
    }
    semctl(sem_id, 0, IPC_RMID);
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    
    if (argc != 2)
    {
        printf("Use command like this: \"./producer file.txt\"\n");
        return 1;
    }
    
    char *file_name = argv[1]; // Получаем название файла из аргументов

    key_t key;
    
    // Создаем файл если он не существует
    FILE *test_file = fopen(file_name, "a");
    if (test_file == NULL)
    {
        perror("fopen create");
        return 1;
    }
    fclose(test_file);
    
    // Генерируем ключ для семафора на основе имени файла
    if ((key = ftok(file_name, 'A')) == -1)
    {
        perror("ftok");
        return 1;
    }
    
    int sem_id;
    // Создаем или получаем семафор
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
    
    printf("Poehali! File: %s, Semaphore ID: %d\n", file_name, sem_id);

    srand(time(NULL));

    while (1)
    {
        // produce_item()
        int num_count = 5 + rand() % 6; // Случайное количество чисел (5-10)
        int numbers[num_count];
        for (int i = 0; i < num_count; i++)
            numbers[i] = rand() % 100; // Случайное число (0-99)

        // Захватываем семафор перед записью в файл
        if (semop(sem_id, &lock, 1) == -1)
        {
            perror("semop lock");
            break;
        }

        // put_item()
        FILE *file = fopen(file_name, "a");
        if (file == NULL)
        {
            perror("fopen");
            semop(sem_id, unlock, 2); // Не забываем разблокировать при ошибке
            break;
        }
        
        for (int i = 0; i < num_count; i++)
            fprintf(file, "%d ", numbers[i]);
        
        fprintf(file, "\n");
        fflush(file); // Важно: сбрасываем буфер сразу
        fclose(file);

        printf("Producer wrote %d numbers to file\n", num_count);

        // Освобождаем семафор
        if (semop(sem_id, unlock, 2) == -1)
        {
            perror("semop unlock");
            break;
        }

        sleep(1); // Пауза между итерациями
    }

    return 0;
}
