#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>

int sem_id = -1;
union semun
{
      int val;
      struct semid_ds *buf;
      unsigned short *array;
      
      struct seminfo *__buf;
};

struct sembuf lock = {0, -1, 0};
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
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    if (argc != 2)
    {
        printf("Use command like this: \"./consumer file.txt\"\n");
        return 1;
    }

    char *file_name = argv[1];
    int sem_id;
    key_t key;

    // Генерируем ключ для семафора на основе имени файла
    if ((key = ftok(file_name, 'A')) == -1)
    {
        perror("ftok");
        return 1;
    }

    // Получаем существующий семафор
    if ((sem_id = semget(key, 1, 0666)) == -1)
    {
        perror("semget");
        printf("Semaphore for file %s not found. Make sure producer is running first.\n", file_name);
        return 1;
    }

    printf("Poehali consume'it! File: %s, Semaphore ID: %d, PID: %d\n", file_name, sem_id, getpid());

    long line_counter = 0;

    while (1)
    {
        // Захватываем семафор перед чтением из файла
        if (semop(sem_id, &lock, 1) == -1)
        {
            perror("semop lock");
            break;
        }

        FILE *file = fopen(file_name, "r");
        if (file == NULL)
        {
            perror("fopen");
            semop(sem_id, unlock, 2);
            sleep(1);
            continue;
        }

        char line[256];
        long current_line = 0;
        int found_new_line = 0;

        // Пропускаем уже обработанные строки
        while (current_line < line_counter && fgets(line, sizeof(line), file) != NULL)
            current_line++;

        // Пытаемся прочитать следующую строку
        //if (fgets(line, sizeof(line), file) != NULL)
        while (fgets(line, sizeof(line), file) != NULL)
        {
            // Обрабатываем новую строку
            int min_val = INT_MAX;
            int max_val = INT_MIN;
            int num;
            char *ptr = line;

            while (sscanf(ptr, "%d", &num) == 1)
            {
                if (num < min_val)
                    min_val = num;
                if (num > max_val)
                    max_val = num;
                while (*ptr && *ptr != ' ' && *ptr != '\n' && *ptr != '\r')
                    ptr++;
                while (*ptr && (*ptr == ' ' || *ptr == '\n' || *ptr == '\r'))
                    ptr++;
            }

            printf("Consumer PID %d: Min=%d, Max=%d (line %ld)\n", getpid(), min_val, max_val, line_counter + 1);
            fflush(stdout);
            
            line_counter++;
            found_new_line = 1;
        }

        fclose(file);

        // Освобождаем семафор
        if (semop(sem_id, unlock, 2) == -1)
        {
            perror("semop unlock");
            break;
        }

        // Регулируем паузу
        sleep(found_new_line ? 1 : 2);
    }

    return 0;
}
