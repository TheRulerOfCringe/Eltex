#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <limits.h>
#include <sys/wait.h>
#include <signal.h>

#define SEM_NAME "/file_semaphore"
int running = 1;

void signal_handler(int sig)
{
    printf("\nParent: Received signal %d, shutting down in couple of seconds, you know. To do this correctly...\n", sig);
    running = 0;
}


int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    if (argc != 2)
    {
        printf("Use: %s file.txt\n", argv[0]);
        return 1;
    }

    char *file_name = argv[1];
    pid_t pid;
    
    // Создаем файл если не существует
    FILE *test_file = fopen(file_name, "a");
    if (test_file == NULL)
    {
        perror("fopen create");
        return 1;
    }
    fclose(test_file);

    // Создаем/открываем POSIX семафор
    // O_CREAT - создать если не существует
    // 0644 - права доступа
    // 1 - начальное значение семафора
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        return 1;
    }

    printf("POSIX semaphore created/opened successfully\n");

    pid = fork();
    
    if (pid == -1)
    {
        perror("fork");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        return 1;
    }

    if (pid > 0)
    {
        // Родитель
        printf("Parent process (Producer) started. PID: %d\n", getpid());
        srand(time(NULL));

        while(running == 1) // Генерируем 10 строк для примера
        {
            // Генерируем строку
            int num_count = 5 + rand() % 6; // 5-10 чисел
            int numbers[num_count];
            for (int j = 0; j < num_count; j++)
            {
                numbers[j] = rand() % 100; // 0-99
            }

            // Захватываем семафор
            if (sem_wait(sem) == -1)
            {
                perror("sem_wait");
                break;
            }

            // Записываем в файл
            FILE *file = fopen(file_name, "a");
            if (file == NULL)
            {
                perror("fopen");
                sem_post(sem);
                break;
            }
            
            for (int j = 0; j < num_count; j++)
            {
                fprintf(file, "%d ", numbers[j]);
            }
            fprintf(file, "\n");
            fflush(file);
            fclose(file);

            printf("Producer wrote %d numbers to file\n", num_count);

            // Освобождаем семафор
            if (sem_post(sem) == -1)
            {
                perror("sem_post");
                break;
            }

            sleep(1); // Пауза между записями
        }

        // Ждем завершения дочернего процесса
        wait(NULL);
        
        // Закрываем и удаляем семафор
        sem_close(sem);
        sem_unlink(SEM_NAME);
        printf("Parent process finished\n");
    }
    else
    {
        // Дочерний процесс
        printf("Child process (Consumer) started. PID: %d\n", getpid());
        
        long line_counter = 0;

        while (1)
        {
            // Захватываем семафор
            if (sem_wait(sem) == -1)
            {
                perror("sem_wait");
                break;
            }

            // Читаем файл
            FILE *file = fopen(file_name, "r");
            if (file == NULL)
            {
                perror("fopen");
                sem_post(sem);
                sleep(1);
                continue;
            }

            char line[256];
            long current_line = 0;
            int found_any_line = 0;

            // Пропускаем обработанные строки
            while (current_line < line_counter && fgets(line, sizeof(line), file) != NULL)
            {
                current_line++;
            }

            // Обрабатываем все новые строки
            while (fgets(line, sizeof(line), file) != NULL)
            {
                int min_val = INT_MAX;
                int max_val = INT_MIN;
                int num;
                char *ptr = line;

                while (sscanf(ptr, "%d", &num) == 1)
                {
                    if (num < min_val) min_val = num;
                    if (num > max_val) max_val = num;
                    while (*ptr && *ptr != ' ' && *ptr != '\n' && *ptr != '\r') ptr++;
                    while (*ptr && (*ptr == ' ' || *ptr == '\n' || *ptr == '\r')) ptr++;
                }

                printf("Consumer: Min=%d, Max=%d (line %ld)\n", min_val, max_val, line_counter + 1);
                line_counter++;
                found_any_line = 1;
            }

            fclose(file);

            // Освобождаем семафор
            if (sem_post(sem) == -1)
            {
                perror("sem_post");
                break;
            }

            // Если новых строк не было, проверяем может быть родитель уже завершился
            if (!found_any_line)
            {
                // Простая проверка - если прошло много времени без новых данных, выходим
                static int empty_checks = 0;
                empty_checks++;
                if (empty_checks > 5) // Если 5 раз подряд не нашли данных
                {
                    printf("Consumer: No new data for long time, exiting\n");
                    break;
                }
                sleep(2);
            }
            else
            {
                sleep(1);
            }
        }

        sem_close(sem);
        printf("Child process finished\n");
    }

    return 0;
}
