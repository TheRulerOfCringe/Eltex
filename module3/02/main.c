#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_INPUT_LENGTH 64
#define MAX_ARGS 10

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

int parse_input(char *input, char *argv[])
{
    int argc = 0;
    char *token = strtok(input, " \t\n");
    
    while (token != NULL && argc < MAX_ARGS - 1)
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL; // NULL at the end
    
    return argc;
}

int main()
{
    char input[MAX_INPUT_LENGTH];
    char *argv[MAX_ARGS];
    
    printf("Welcome to my interpreter!\nType 'exit' to quit\n\n");
    bool exit = true;
    
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
        int argc = parse_input(input, argv);
        
        // If net argumentov, then vsyo po novoy
        if (argc == 0)
            continue;
        
        // Na BbIXOD
        if (strcmp(argv[0], "exit") == 0)
        {
            exit = false;
            continue;
        }
        
        // Executim commandu
        execute_command(argv);
    }
    
    return 0;
}
