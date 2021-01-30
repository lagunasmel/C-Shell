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

// command [arg1 arg2 ...] [< input_file] [> output_file] [&]
struct userCommand
{
    char *command;
    char *args[MAX_ARG_NUM + 1];
    char *inputFile;
    char *outputFile;
    // int *exeInBackground; // set to false initially
};

/* Tokenize the user command and create struct from it */
struct userCommand *tokenizeCommand(char *input)
{
    /* Format of incoming input is: 
        command [arg1 arg2 ...] [< input_file] [> output_file] [&] */
    char *inputSymbol = "<";
    char *outputSymbol = ">";
    char *exeCommand = "&\n";

    struct userCommand *currCommand = malloc(sizeof(struct userCommand));

    /* Use with strtok_r */
    char *saveptr;
    char *argptr;

    //*currCommand->exeInBackground = 0;

    /* Grab the command */
    char *token = strtok_r(input, " ", &saveptr);
    currCommand->command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currCommand->command, token);

    /* Grab any optional args */
    int i = 0;

    while (token = strtok_r(NULL, " ", &saveptr))
    {
         /* Check for any input files */
        if (strcmp(inputSymbol, token) == 0) 
        {
        /* If we have found the < symbol, forward the pointer 
            to save the input file name */
            token = strtok_r(NULL, " ", &saveptr);
            currCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCommand->inputFile, token);
        }
        /* Check for any output files */
        else if (strcmp(outputSymbol, token) == 0) 
        {
            /* If we have found the > symbol, forward the pointer 
            to save the output file name */
            token = strtok_r(NULL, " ", &saveptr);
            currCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCommand->outputFile, token);
        }
        /* Check if this command will be executed in the background */
        else if (strcmp(exeCommand, token) == 0) 
        {
            // *currCommand->exeInBackground = 1;
            break; /* we can exit the loop here */
        }
        /* Otherwise, we are saving as an arg */
        else
        {
            /* If we have not encountered symbols, it is an arg */
            currCommand->args[i] = token;
            i++;
        }
    }

    return currCommand;
};

void printCurrCommand(struct userCommand *aUserCommand)
{
    printf("command is %s\n", aUserCommand->command);
}

void printArgs(struct userCommand *aUserCommand)
{
    for (int i = 0; i <= MAX_ARG_NUM; i++)
    {
        if (aUserCommand->args[i] != NULL)
        {
            printf("Arg is %s\n", aUserCommand->args[i]);
        }
    }
}

void printInputFileName(struct userCommand *aUserCommand)
{
    printf("Input file name is %s\n", aUserCommand->inputFile);
}

void printOutputFileName(struct userCommand *aUserCommand)
{
    printf("Output file name is %s\n", aUserCommand->outputFile);
}

int printBoolean(struct userCommand *aUserCommand)
{
    //printf("Boolean is %d\n", aUserCommand->exeInBackground);
    return 0;
}
/* Receives input for the user. Will parse the input. */
int getInput()
{

    char *buffer;
    size_t bufsize = 0;
    getline(&buffer, &bufsize, stdin);
    fflush(stdout);

    char *newLine = "\n";
    char *hash = "#";
    char *exitCommand = "exit\n";

    /* Create a struct from the current command */
    struct userCommand *currCommand = tokenizeCommand(buffer);

    printCurrCommand(currCommand);
    printArgs(currCommand);
    printInputFileName(currCommand);
    // printOutputFileName(currCommand);
    // printBoolean(currCommand);

    free(buffer);


    /* Ignore # and new lines */

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

    return 0;
}