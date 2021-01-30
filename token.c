#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#define MAX_CHAR_LENGTH 2048
#define MAX_ARG_NUM 512

int main()
{
    char *saveptr;
    // char arg[] = "command -l arg1 arg2 < input_file > output_file &";

    char *arg;
    size_t bufsize = 0;
    printf("enter input: ");
    getline(&arg, &bufsize, stdin);
    fflush(stdout);

    // printf("%s\n", arg);

    char *token = strtok_r(arg, " ", &saveptr);

    printf("current token is %s\n", token);

    char *inputSymbol = "<";
    char *outputSymbol = ">";
    char *exeCommand = "&\n";

    while (token = strtok_r(NULL, " ", &saveptr))
    {
        if (token != NULL)
        {
            printf("arg found: %s\n", token);
            if (strcmp(inputSymbol, token) == 0)
            {
                // printf("input found");
                break;
            }
        }
    }

    token = strtok_r(NULL, " ", &saveptr);
    printf("input or output is: %s\n", token);

    return 0;
}