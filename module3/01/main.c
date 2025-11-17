#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>  // для isdigit()
#include <unistd.h>
#include <stdlib.h>

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
    printf("WHO PRINT THAT?\n");
    int mid = (argc+1)/2;
	pid_t pid;
	pid = fork();
	if (pid < 0)
	{
	    printf("Error, pid < 0!!");
	}
	else
	{
	    if (pid > 0)
	    {
		    for (int i = 1; i < mid; i++)
		    {
		        //printf("parent, %d %s\n", i, argv[i]); // родительский процесс
		        if (is_int(argv[i]))
		        {
		            int tmp = atoi(argv[i]);
		            printf("paretn said: %d) %d, %d\n", i, tmp, tmp*2);
		        }
		        else if (is_float(argv[i]))
		        {
		            float tmp = atof(argv[i]);
		            printf("paretn said: %d) %f, %f\n", i, tmp, tmp*2);
		        }
		        else
		        {
		            printf("paretn said: %d) %s\n", i, argv[i]);
		        }
		    }
	    }
	    else if (pid == 0)
	    {
		    for (int i = mid; i < argc; i++)
		    {
		        if (is_int(argv[i]))
		        {
		            int tmp = atoi(argv[i]);
		            printf("child said: %d) %d, %d\n", i, tmp, tmp*2);
		        }
		        else if (is_float(argv[i]))
		        {
		            float tmp = atof(argv[i]);
		            printf("child said: %d) %f, %f\n", i, tmp, tmp*2);
		        }
		        else
		        {
		            printf("child said: %d) %s\n", i, argv[i]);
		        }
		    }
	    }
	    else
	    {
		    printf("Error!");
	    }
	    printf("\nMy pid is %d\n", pid);
	}
}
