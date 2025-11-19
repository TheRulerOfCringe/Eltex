#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_INPUT_LENGTH 64
#define MAX_ARGS 10

int execute_commands(char *args1[], char *args2[])
{
    int pipefd[2];
    pid_t pid1, pid2;
    
    // Creating pipe
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return -1;
    }
    
    // First command - writes to pipe
    pid1 = fork();
    if (pid1 == 0)
    {
        close(pipefd[0]);  // Close reading pipe
        
        // reroute stdout na zapis v pipe
        dup2(pipefd[1], 1);
        close(pipefd[1]);
        
        execvp(args1[0], args1);
        perror(args1[0]);
        exit(127);
    }
    
    // ВFirst command - reads from pipe
    pid2 = fork();
    if (pid2 == 0)
    {
        close(pipefd[1]);  // Close zapis pipe
        
        // reroute stdin na reading s pipa
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        
        execvp(args2[0], args2);
        perror(args2[0]);
        exit(127);
    }
    
    // Parent close pipes
    close(pipefd[0]);
    close(pipefd[1]);
    
    // Paretn waits
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    
    return 0;
}

int execute_command(char *argv[])
{
    pid_t pid = fork();
    
    if (pid == -1)
    {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0)
    {
        // Docherniy process
        execvp(argv[0], argv);
        
        // execvp in case of success zavershitsya, nishe kod ne poydet
        perror(argv[0]);
        exit(127); // "command not found"
    }
    else
    {
        
        // wait for Docherniy ending and smotrim na oshibki
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid failed");
            return -1;
        }
        if (WIFEXITED(status))
        {
            const int es = WEXITSTATUS(status);
            printf("exit status was %d\n", es);
        }
    }
    return 0;
}

bool parse_input(char *input, char *argv1[], int *argc1, char *argv2[], int *argc2, bool *no_pipe)
{
    *no_pipe = true;
    *argc1 = 0;
    *argc2 = 0;
    char *token = strtok(input, " \t\n");
    
    while (token != NULL && *argc1 < MAX_ARGS - 1)
    {
        if (strcmp(token, "|") == 0)
            {
                *no_pipe = false;
                break;
            }
        argv1[*argc1] = token;
        (*argc1)++;
        token = strtok(NULL, " \t\n");
    }
    argv1[*argc1] = NULL; // NULL at the end
    token = strtok(NULL, " \t\n");
    
    while (token != NULL && *argc2 < MAX_ARGS - 1)
    {
        if (strcmp(token, "|") == 0)
            return false;
        argv2[*argc2] = token;
        (*argc2)++;
        token = strtok(NULL, " \t\n");
    }
    argv2[*argc2] = NULL; // NULL at the end
    
    return true;
}

// For perenapravlenie
void setup_redirection(char *input_file, char *output_file, bool append)
{
    // For input
    if (input_file != NULL)
    {
        int fd = open(input_file, O_RDONLY);
        if (fd == -1)
        {
            perror(input_file);
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // For output
    if (output_file != NULL)
    {
        int flags = O_WRONLY | O_CREAT;
        if (append)
            flags |= O_APPEND;  // For >>
        else
            flags |= O_TRUNC;   // For >
        
        int fd = open(output_file, flags, 0666);
        if (fd == -1)
        {
            perror(output_file);
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

int execute_command2(char *argv[], char *input_file, char *output_file, bool append)
{
    pid_t pid = fork();
    
    if (pid == -1)
    {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0)
    {
        // Docherniy process
        setup_redirection(input_file, output_file, append);
        execvp(argv[0], argv);
        
        // execvp in case of success zavershitsya, nishe kod ne poydet
        perror(argv[0]);
        exit(127); // "command not found"
    }
    else
    {
        // wait for Docherniy ending and smotrim na oshibki
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid failed");
            return -1;
        }
        if (WIFEXITED(status))
        {
            const int es = WEXITSTATUS(status);
            printf("exit status was %d\n", es);
        }
    }
    return 0;
}

bool parse_input2(char *input, char *argv1[], int *argc1, char **input_file1, char **output_file1, bool *append1, char *argv2[], int *argc2, char **input_file2, char **output_file2, bool *append2, bool *no_pipe)
{
    *no_pipe = true;
    *argc1 = 0;
    *argc2 = 0;
    *input_file1 = NULL;
    *output_file1 = NULL;
    *append1 = false;
    *input_file2 = NULL;
    *output_file2 = NULL;
    *append2 = false;
    
    char *token = strtok(input, " \t\n");
    
    // Парсим первую команду
    while (token != NULL && *argc1 < MAX_ARGS - 1)
    {
        if (strcmp(token, "|") == 0)
        {
            *no_pipe = false;
            break;
        }
        else if (strcmp(token, "<") == 0)
        {
            // Ввод из файла
            token = strtok(NULL, " \t\n");
            if (token == NULL)
            {
                fprintf(stderr, "Error: no file after '<'\n");
                return false;
            }
            *input_file1 = token;
        }
        else if (strcmp(token, ">") == 0)
        {
            // Вывод в файл (перезапись)
            token = strtok(NULL, " \t\n");
            if (token == NULL)
            {
                fprintf(stderr, "Error: no file after '>'\n");
                return false;
            }
            *output_file1 = token;
            *append1 = false;
        }
        else if (strcmp(token, ">>") == 0)
        {
            // Вывод в файл (добавление)
            token = strtok(NULL, " \t\n");
            if (token == NULL)
            {
                fprintf(stderr, "Error: no file after '>>'\n");
                return false;
            }
            *output_file1 = token;
            *append1 = true;
        }
        else
        {
            // Обычный аргумент команды
            argv1[(*argc1)++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    argv1[*argc1] = NULL;
    
    // Если есть пайп, парсим вторую команду
    if (!(*no_pipe))
    {
        token = strtok(NULL, " \t\n");
        while (token != NULL && *argc2 < MAX_ARGS - 1)
        {
            if (strcmp(token, "|") == 0)
            {
                fprintf(stderr, "Error: multiple pipes not supported\n");
                return false;
            }
            else if (strcmp(token, "<") == 0)
            {
                // Ввод из файла для второй команды
                token = strtok(NULL, " \t\n");
                if (token == NULL)
                {
                    fprintf(stderr, "Error: no file after '<'\n");
                    return false;
                }
                *input_file2 = token;
            }
            else if (strcmp(token, ">") == 0)
            {
                // Вывод в файл для второй команды (перезапись)
                token = strtok(NULL, " \t\n");
                if (token == NULL)
                {
                    fprintf(stderr, "Error: no file after '>'\n");
                    return false;
                }
                *output_file2 = token;
                *append2 = false;
            }
            else if (strcmp(token, ">>") == 0)
            {
                // Вывод в файл для второй команды (добавление)
                token = strtok(NULL, " \t\n");
                if (token == NULL)
                {
                    fprintf(stderr, "Error: no file after '>>'\n");
                    return false;
                }
                *output_file2 = token;
                *append2 = true;
            }
            else
            {
                // Обычный аргумент команды
                argv2[(*argc2)++] = token;
            }
            token = strtok(NULL, " \t\n");
        }
        argv2[*argc2] = NULL;
    }
    
    return true;
}

int main()
{
    char input[MAX_INPUT_LENGTH];
    char *argv1[MAX_ARGS], *argv2[MAX_ARGS];
    int argc1 = 0, argc2 = 0;
    bool exit = true, no_pipe = true;
    // Dlya perenapravleniy
    char *input_file1 = NULL, *output_file1 = NULL;
    char *input_file2 = NULL, *output_file2 = NULL;
    bool append1 = false, append2 = false;
    
    printf("Welcome to my interpreter!\nType 'exit' to quit\n\n");
    
    while (exit)
    {
        // Invitation
        printf("Interpreter> ");
        fflush(stdout); // So user will see invitation before input
        
        // Reading input
        if (fgets(input, MAX_INPUT_LENGTH, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        
        // If pustaya stroka, then vsyo po novoy
        if (strlen(input) == 0)
            continue;
        
        // Parsing input
        if (!parse_input2(input, argv1, &argc1, &input_file1, &output_file1, &append1, argv2, &argc2, &input_file2, &output_file2, &append2, &no_pipe))
            printf("Error in nesting pipes occure!\n");
        else
        {
            // If net argumentov, then vsyo po novoy
            if (argc1 == 0)
                continue;
            if (!no_pipe && argc2 == 0)
                continue;
            
            // Na BbIXOD
            if (strcmp(argv1[0], "exit") == 0)
            {
                exit = false;
                continue;
            }
            
            // Executim commandu
            if (no_pipe)
                execute_command2(argv1, input_file1, output_file1, append1);
            else
                execute_commands(argv1, argv2);
        }
        argc1 = 0;
        argc2 = 0;
        input_file1 = NULL;
        output_file1 = NULL;
        append1 = false;
        input_file2 = NULL;
        output_file2 = NULL;
        append2 = false;
    }
    
    printf("Poka-poka!\n");
    return 0;
}
