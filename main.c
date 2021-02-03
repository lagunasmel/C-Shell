#include <stdio.h>
#include <stdlib.h>
#include <string.h>    /* strcmp, strcat */
#include <stdbool.h>   /* bool */
#include <sys/types.h> /* pid */
#include <unistd.h>    /* chdir, exec, dup2 */
#include <sys/wait.h>  /* waitpid */
#include <fcntl.h>     /* fcntl */

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

/* Detects the presence of $$ variable and returns true or false */
bool detectExpVar(char *input)
{
    char *expandVariable = "$$";

    char *currStr;
    currStr = strstr(input, expandVariable);

    /* Check if the string is null. Return false if so. */
    if (currStr == NULL)
    {
        return false;
    }
    else
    {
        /* Return true if currStr is not null */
        return true;
    }
}

/* Counts the number of occurrences of the $$ variable. Returns an int of the count. */
int countExpVar(char *string)
{
    char *expVariable = "$";
    char varCount = 0;
    char totalCount = 0;

    /* Traverse through the string and count any $$ that appear twice */
    for (int i = 0; i < strlen(string); i++)
    {
        /* Exclude counting any $ is not consecutive before or after another $*/
        if (string[i] == *expVariable && (string[i - 1] == *expVariable || string[i + 1] == *expVariable))
        {
            varCount++;
            if (varCount == 2)
            {
                /* Reset the varCount when we count 2 consecutive $ */
                varCount = 0;
                totalCount++;
            }
        }
    }

    return totalCount;
}

/* Receives: the old string and the pid */
/* Returns: the modified string, replacing all instances of the expansion variable 
with the pid */
char *replaceExpVar(char *oldStr, char *pid)
{
    char *expVariable = "$";
    int i;                               /* used to iterate through old string */
    int j = 0;                           /* used to iterate and build new string */
    int pidLength = strlen(pid);         /* calculates how long the PID string length is */
    int numOfVars = countExpVar(oldStr); /* counts number of times $$ appears */

    /* Create a new char pointer */
    char *modStr;
    /* The length of the modified string is calculated by the count and pidLength. 
    We add the pidlength and multiply it by the count of $$ variable, then subtract
    the count * 2 to subtract the two $$ symbols */
    modStr = calloc(strlen(oldStr) + (numOfVars * pidLength) - (numOfVars * 2) + 1, sizeof(char));

    /* Iterate through the old string to append to the new string */
    for (i = 0; i < strlen(oldStr); i++)
    {
        /* If we find a pair of $$, we can replace the symbols with the pid here */
        if (oldStr[i] == *expVariable && oldStr[i + 1] == *expVariable)
        {
            strcat(modStr, pid); /* Concatenate the pid string to the current index */
            j += pidLength;      /* forward the index in the new string */
            i++;                 /* forward the index in the old string to avoid counting duplicates */
        }
        else
        {
            /* Otherwise, we can proceed normally with adding the string char by char */
            modStr[j] = oldStr[i];
            j++;
        }
    }

    return modStr;
}

/* Tokenize the user command and create struct from it */
struct userCommand *tokenizeCommand(char *input)
{
    /* Format of incoming input:
    command [arg1 arg2 ...] [< input_file] [> output_file] [&] */
    char *inputSymbol = "<";
    char *outputSymbol = ">";
    char *exeCommand = "&";

    char *modifiedStr; /* size to be determined according to presence of $$ variable */

    /* Check if the $$ is present in the input */
    bool varDetected = detectExpVar(input);

    /* if the expansion variable is detected, modify the string accordingly */
    if (varDetected)
    {
        /* Grab the pid and convert it to a string */
        char *pid;
        int pidInt = getpid();
        pid = calloc(10, sizeof(char));
        sprintf(pid, "%d", pidInt);
        /* Use the pid length and expansion variable count to allocate memory */
        int pidLength = strlen(pid);
        int numOfVars = countExpVar(input);

        modifiedStr = calloc(strlen(input) + (numOfVars * pidLength) - (numOfVars * 2) + 1, sizeof(char));

        /* Replace all $$ in the input string */
        modifiedStr = replaceExpVar(input, pid);
    }
    /* Otherwise, we can proceed with the string without modifying it */
    else
    {
        /* Copy the user input to the modifiedStr variable */
        modifiedStr = calloc(strlen(input) + 1, sizeof(char));
        strcpy(modifiedStr, input);
    }

    /* Remove the trailing new line character*/
    /* Code snippet for the following line of code borrowed from here: 
    https://stackoverflow.com/questions/9628637/how-can-i-get-rid-of-n-from-string-in-c*/
    modifiedStr[strcspn(modifiedStr, "\n")] = '\0';

    struct userCommand *currCommand = malloc(sizeof(struct userCommand));

    /* Check if the command needs to be executed in the background */
    int len = strlen(modifiedStr);
    if (modifiedStr[len - 1] == *exeCommand)
    {
        /* Set to true if the & symbol is detected */
        currCommand->exeInBackground = true;
        /* Remove it so it doesn't get parsed */
        modifiedStr[len - 1] = '\0';
    }

    char *exitCommand = "exit";
    char *statusCommand = "status";
    char *lsCommand = "ls";

    /* The following code snippet was modeled after the parsing input example provided here: 
    https://repl.it/@cs344/studentsc#main.c */
    /* It is used to tokenize the user input and store the input in a struct to hold 
    the command, any arguments, input file, outfile, and whether to run the process in the foreground
    or background. */

    /* Use with strtok_r */
    char *saveptr;

    /* Grab the command */
    char *token = strtok_r(modifiedStr, " ", &saveptr);
    currCommand->command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currCommand->command, token);

    /* Check for status flags and return them immediately if they are detected */
    if (strcmp(currCommand->command, exitCommand) == 0)
    {
        return currCommand;
    }

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
    fflush(stdout);
}

/* Used for testing purposes for userCommand struct */
void printArgs(struct userCommand *aUserCommand)
{
    for (int i = 0; i <= MAX_ARG_NUM; i++)
    {
        if (aUserCommand->args[i] != NULL)
        {
            printf("Arg is %s\n", aUserCommand->args[i]);
            fflush(stdout);
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
        printf("boolean is false\n");
    }
}
/* setOutput */
/* Receives: the userCommand struct */
/* Sets output file if it is detected in the command. Prints any error messages. */
/* Returns: void */
/* This function was modeled after the code snippet presented here:
https://repl.it/@cs344/54redirectc */
void setOutput(struct userCommand *currCommand)
{
    int targetFD;

    /* Check if there is an output file in the struct */
    if (currCommand->outputFile != NULL)
    {
        targetFD = open(currCommand->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);

        if (targetFD == -1)
        {
            perror("Error opening file");
            exit(1);
        }

        // Currently printf writes to the terminal
        printf("The file descriptor for targetFD is %d\n", targetFD);
        fflush(stdout);

        // Point FD 1 to the target FD (outputFile)
        int result = dup2(targetFD, 1);
        if (result == -1)
        {
            perror("dup2");
            exit(1);
        }
    }
}

int freeChild(struct userCommand *currCommand)
{
    // int i = 0;
    free(currCommand->command);

    for (int i = 0; i <= MAX_ARG_NUM; i++)
    {
        free(currCommand->args[i]);
    }
    free(currCommand->inputFile);
    free(currCommand->outputFile);
    currCommand->exeInBackground = false;
    return 0;
}

/* countArgs */
/* Counts the number of arguments currently in the user command */
/* Receives: userCommand struct */
/* Returns: an int indicating the number of arguments */
int countArgs(struct userCommand *currCommand)
{
    int count = 0;
    int i = 0;
    while (currCommand->args[i] != NULL)
    {
        count++;
        i++;
    }
    return count;
}

/* createChildProcess */
/* This function was modeled after an example fork provided here: 
https://repl.it/@cs344/42execlforklsc 
It was featured in the "Executing a New Program" in Module 4. */
void createChildProcess(struct userCommand *currCommand)
{
    int childStatus;

    /* Retrieve number of arguments in the current command */
    int argCount = countArgs(currCommand);

    /* Initialize args array to be passed to execvp */
    char *args[MAX_ARG_NUM] = {NULL};
    args[0] = currCommand->command; /* this contains the command to be executed */

    int i = 1; /* counter for the args array */
    int j = 0; /* counter to iterate through the command line arguments */

    /* Add all of the arguments from the user command to the args array */
    while (currCommand->args[j] != NULL)
    {
        args[i] = currCommand->args[j];
        j++;
        i++;
    }

    // Fork a new process
    pid_t spawnPid = fork();

    switch (spawnPid)
    {
    case -1:
        perror("fork()\n");
        exit(1);
        break;
    case 0:
        /* Detect an input file */

        /* Detect any output file */
        setOutput(currCommand);

        /* Execute the command */
        execvp(args[0], args);

        /* Return if there is an error */
        perror(args[0]);
        freeChild(currCommand);
        exit(1);
        break;
    default:
        // In the parent process
        // Wait for child's termination
        spawnPid = waitpid(spawnPid, &childStatus, 0);
        //printf("PARENT(%d): child(%d) terminated. Exiting function\n", getpid(), spawnPid);
        //fflush(stdout);
        //exit(0);
        break;
    }
}

/* processCommand */
/* Receives: the usercommand struct 
Checks if the command raises any of the 3 built in commands (cd, status, or exit)
If not, it is a child process, and is processed accordingly.
Returns: VOID */
void processCommand(struct userCommand *currCommand)
{
    char *exitFlag = "exit";
    char *cdFlag = "cd";
    char *statusFlag = "status";
    /* Check for any status flags */

    /* Check if the user wants to exit - need to complete */
    if (strcmp(currCommand->command, exitFlag) == 0)
    {
        /* Kill any child processes */
        free(currCommand);
        exit(0);
    }
    /* If the user wants to change directories */
    else if (strcmp(currCommand->command, cdFlag) == 0)
    {
        /* Check if first arg is null. If so, change to the directory in the HOME variable */
        if (currCommand->args[0] == NULL)
        {
            int changeDir = chdir(getenv("HOME"));
        }
        /* Otherwise, change to the path it's being directed to */
        else
        {
            int changeDir = chdir(currCommand->args[0]);

            if (changeDir == -1)
            {
                printf("No directory named %s found in the current directory\n", currCommand->args[0]);
                fflush(stdout);
            }
        }
        /* FOR TESTING PURPOSES -  comment out later */
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("cwd is %s\n", cwd);
            fflush(stdout);
        }
    }
    /* Check if the user entered the status flag */
    else if (strcmp(currCommand->command, statusFlag) == 0)
    {
        printf("status flag raised\n");
        fflush(stdout);
    }
    /* Otherwise, this is a child processs */
    else
    {
        createChildProcess(currCommand);
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

    /* Check if it's a new line or hash, and ignore if so */
    if (strcmp(buffer, newLine) == 0 || buffer[0] == *hash)
    {
        free(buffer);
        return 0;
    }
    /* If it is not a hash or new line, we can make a struct from the current command */
    struct userCommand *currCommand = tokenizeCommand(buffer);

    processCommand(currCommand);

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
        fflush(stdout);
        int showPrompt = getInput();
    }

    return 0;
}