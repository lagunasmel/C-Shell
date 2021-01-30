#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CHAR_LENGTH 2048
#define MAX_ARG_NUM 512

/* Displays the shell prompt for user to enter chars and args */
void displayPrompt()
{
    printf(": ");
};

/* Struct that formats and stores the input received by the user. */
struct userCommand
{
    char *command;
    char *args[MAX_ARG_NUM + 1];
    char *inputFile;
    char *outputFile;
    bool exeInBackground; /* Initially set to false */
};

/* Tokenize the user command and create struct from it */
struct userCommand *tokenizeCommand(char *input)
{
    /* Format of incoming input is: 
        command [arg1 arg2 ...] [< input_file] [> output_file] [&] */
    char *inputSymbol = "<";
    char *outputSymbol = ">";
    char *exeCommand = "&\n";
    char *exeCommand2 = "&";

    struct userCommand *currCommand = malloc(sizeof(struct userCommand));

    /* Use with strtok_r */
    char *saveptr;
    char *argptr;

    /* Grab the command */
    char *token = strtok_r(input, " ", &saveptr);
    currCommand->command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currCommand->command, token);

    /* Counter to store any optional args */
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
        else if (strcmp(exeCommand, token) == 0 || strcmp(exeCommand2, token) == 0)
        {
            currCommand->exeInBackground = true;
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

/* Used for testing purposes for userCommand struct */
void printCurrCommand(struct userCommand *aUserCommand)
{
    printf("command is %s\n", aUserCommand->command);
}

/* Used for testing purposes for userCommand struct */
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

/* Used for testing purposes for userCommand struct */
void printInputFileName(struct userCommand *aUserCommand)
{
    printf("Input file name is %s\n", aUserCommand->inputFile);
}

/* Used for testing purposes for userCommand struct */
void printOutputFileName(struct userCommand *aUserCommand)
{
    printf("Output file name is %s\n", aUserCommand->outputFile);
}

/* Used for testing purposes for userCommand struct */
void printBoolean(struct userCommand *aUserCommand)
{
    if (aUserCommand->exeInBackground)
    {
        printf("Boolean is true\n");
    }
    else
    {
        printf("boolean is false");
    }
}

/* Receives input from the user. */
/* Will parse the input and store it in a userCommand struct. */
int getInput()
{
    char *buffer;
    size_t bufsize = 0;
    getline(&buffer, &bufsize, stdin);

    char *newLine = "\n";
    char *hash = "#";
    char *exitCommand = "exit\n";

    /* Check if it's a new line or hash, and ignore if so */
    if (strcmp(buffer, newLine) == 0 || buffer[0] == *hash)
    {
        free(buffer);
        return 0;
    }
    /* If it is not a hash or new line, we can make a struct from the current command */
    struct userCommand *currCommand = tokenizeCommand(buffer);

    /* Used for testing struct input is correct */
    // printCurrCommand(currCommand);
    // printArgs(currCommand);
    // printInputFileName(currCommand);
    // printOutputFileName(currCommand);
    // printBoolean(currCommand);

    free(buffer);
    return 0;
}

int main()
{
    int userInput = 0;

    while (userInput == 0)
    {
        /* Display the shell prompt */
        displayPrompt();
        int showPrompt = getInput();
        fflush(stdout);
    }

    return 0;
}