#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>  // для isdigit()
#include <unistd.h>

bool is_int(const char* str)
{
    if (str == NULL || *str == '\0')
		return false;
	
    int i = 0; // выбирал между копией указателя и просто i. i победил
	
    if (str[i] == '+' || str[i] == '-')
		i++;
    
    for (; str[i] != '\0'; i++)
        if (!isdigit(str[i]))
			return false;
    
    return true;
}

bool is_float(const char* str)
{
    if (str == NULL || *str == '\0')
		return false;
	
    int i = 0;
	bool flag = true;
	
    if (str[i] == '+' || str[i] == '-')
		i++;
	
    for (; str[i] != '\0'; i++)
	{
        if (!isdigit(str[i]) && !(str[i] == '.'))
			return false;
		
		if (str[i] == '.')
		{
			if (flag)
				flag = false;
			else
				return false;
		}	
	}
	
	return true;
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
	{
        printf("Arg %d: \"%s\". Is_int = %d. Is_float = %d.\n", i, argv[i], is_int(argv[i]), is_float(argv[i]));
    }
	pid_t pid;
	pid = fork();
	if (pid > 0)
	{
		; // родительский процесс
	}
	else if (pid == 0)
	{
		; // дочерний процесс
	}
	else
	{
		;
	}
	printf("%d", pid);
}