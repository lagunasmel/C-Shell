#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR_LENGTH 2048
#define MAX_ARG_NUM 512

/* Displays the shell prompt for user to enter chars and args */
void displayPrompt()
{
    printf(": ");
};

int getInput()
{
    char *buffer;
    size_t bufsize = 0;
    getline(&buffer, &bufsize, stdin);
    fflush(stdout);

    char *newLine = "\n";
    char *hash = "#\n";
    char *exit = "exit\n";

    if ((strcmp(newLine, buffer) == 0) || (strcmp(hash, buffer) == 0))
    {
        // printf("command ignored\n");
        return -1;
    } 
    else if (strcmp(exit, buffer) == 0)
    {
        return 1;
    }

    return 0;
}

int main()
{
    int userInput = 0;

    while (userInput == 0)
    {
        displayPrompt();
        int showPrompt = getInput();

        if (showPrompt == 1)
        {
            userInput = 1;
        }
    }

    return (0);
}