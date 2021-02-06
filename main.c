#include <stdio.h>
#include <stdlib.h>
#include <string.h>    /* strcmp, strcat */
#include <stdbool.h>   /* bool */
#include <sys/types.h> /* pid */
#include <unistd.h>    /* chdir, exec, dup2 */
#include <sys/wait.h>  /* waitpid */
#include <fcntl.h>     /* fcntl */
#include <signal.h>

#define MAX_CHAR_LENGTH 2048
#define MAX_ARG_NUM 512

bool fgOnlyMode = false;

/* Displays the shell prompt for user to enter chars and args */
void displayPrompt()
{
    printf(": ");
    fflush(stdout);
};

/* Struct that formats and stores the input received by the user. */
struct userCommand
{
    char *command;
    char *args[MAX_ARG_NUM];
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
struct userCommand *parseCommand(char *input)
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
        if (!fgOnlyMode)
        {
            currCommand->exeInBackground = true;
        }
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
/* redirectOutput */
/* Receives: the userCommand struct */
/* Sets output file if it is detected in the command. Prints any error messages. */
/* Returns: void */
/* This function was modeled after the code snippet presented here:
https://repl.it/@cs344/54redirectc */
void redirectOutput(struct userCommand *currCommand)
{
    int targetFD;

    /* Check if there is an output file in the struct */
    if (currCommand->outputFile != NULL)
    {
        targetFD = open(currCommand->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        /* Check for any errors */
        if (targetFD == -1)
        {
            perror("Error opening file: ");
            exit(1);
        }

        /* Point FD 1 to the target FD (outputFile) */
        int result = dup2(targetFD, 1);
        if (result == -1)
        {
            perror("Error redirecting the output: ");
            exit(1);
        }
    }
    /* Check if the user doesn't redirect the standard output for a background command */
    else if (currCommand->exeInBackground && currCommand->outputFile == NULL)
    {
        targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    return;
}

/* redirectInput */
/* Receives: the userCommand struct */
/* Sets input file if it is detected in the command. Prints any error messages. */
/* Returns: void */
/* This function was modeled after the code snippet presented here:
/* https://repl.it/@cs344/54sortViaFilesc */
void redirectInput(struct userCommand *currCommand)
{
    int sourceFD;

    /* Open the input file if it has been detected */
    if (currCommand->inputFile != NULL)
    {
        /* open the source file */
        sourceFD = open(currCommand->inputFile, O_RDONLY, 0644);
        /* Check for errors */
        if (sourceFD == -1)
        {
            perror("Error opening input file: ");
            exit(1);
        }
        /* Redirect the input file */
        int result = dup2(sourceFD, 0);
        /* Check for errors */
        if (result == -1)
        {
            perror("Error redirecting the input: ");
            exit(1);
        }
    }
    else if (currCommand->exeInBackground && currCommand->inputFile == NULL)
    {
        sourceFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    return;
}

/* Frees all of the data in the currCommand after it has been processed. */
int freeCommand(struct userCommand *currCommand)
{
    free(currCommand->command);

    for (int i = 0; i <= MAX_ARG_NUM; i++)
    {
        free(currCommand->args[i]);
    }
    free(currCommand->inputFile);
    free(currCommand->outputFile);
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

int returnNextIndex(int processIDs[])
{
    int count = 0;

    for (int i = 0; i < 200; i++)
    {
        if (processIDs[i] == 0)
        {
            break;
        }
        count++;
    }
    return count;
}
/* getStatus */
/* Checks the exit status of the child. 
Will indicate whether the status was terminated by a signal. 
This function must only be used by the parent branch. */
void getStatus(int childStatus, int exitStatus[])
{
    /* If exited normally*/
    if (WIFEXITED(childStatus))
    {
        exitStatus[0] = WEXITSTATUS(childStatus);
    }
    else if (WIFSIGNALED(childStatus))
    {
        exitStatus[0] = WTERMSIG(childStatus);
        printf("terminated by signal %d\n", exitStatus[0]);
        fflush(stdout);
    }
    return;
}

/* createChildProcess */
/* This function was modeled after an example fork provided here: 
https://repl.it/@cs344/42execlforklsc 
It was featured in the "Executing a New Program" in Module 4. */
void createChildProcess(struct userCommand *currCommand, int processIDs[], int exitStatus[],
                        struct sigaction SIGINT_action, struct sigaction SIGTSTP_action)
{
    int childStatus;
    /* Used to retrieve the next available index if the command is a bg process */
    int indexNum;

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

    // Fork a new child process
    pid_t spawnPid = fork();

    switch (spawnPid)
    {
    case -1:
        perror("fork() error\n");
        exit(1);
        break;
    case 0:
        /* Child process */
        /* Set up signal handler for child in fg */
        if (!currCommand->exeInBackground)
        {
            /* signal handlers */
            SIGINT_action.sa_handler = SIG_DFL; /* set default action back */
            sigfillset(&SIGINT_action.sa_mask);
            SIGINT_action.sa_flags = 0;
            sigaction(SIGINT, &SIGINT_action, NULL);
        }
        /* All children must ignore ctrl+z */
        SIGTSTP_action.sa_handler = SIG_IGN;
        sigaction(SIGTSTP, &SIGTSTP_action, NULL);

        /* Detect an input file */
        redirectInput(currCommand);
        /* Detect any output file */
        redirectOutput(currCommand);
        /* Execute the command */
        execvp(args[0], args);

        /* Return if there is an error */
        perror(args[0]);
        freeCommand(currCommand);
        exit(1);
        break;
    default:
        /* Parent process */
        /* Wait for the process to finish executing if in the foreground */
        if (!currCommand->exeInBackground)
        {
            spawnPid = waitpid(spawnPid, &childStatus, 0);
            /* check exit status and print if necessary */
            getStatus(childStatus, exitStatus);
        }
        else
        {
            /* Otherwise, this is a background process */
            indexNum = returnNextIndex(processIDs); /* find the next available index */
            processIDs[indexNum] = spawnPid;        /* assign the new process ID to this index */
            printf("background pid is %d\n", spawnPid);
            fflush(stdout);
        }
        break;
    }
}

/* checkBackgroundProcs */
/* Receives: list of process IDs running in the background
This function will iterate through the list and check for any IDs higher than 0. 
Any number above 0 indicates that it requires checking for finished status. 
/* Returns: an int */
int checkBackgroundProcs(int processIDs[])
{

    int childStatus;       /* for the waitpid */
    int terminationStatus; /* for checking the status of child */
    int childPid;          /* id of child we are checking */

    /* If there are no background processes running, we can exit */
    if (processIDs[0] == 0)
    {
        return 1;
    }
    /* Retrieve the number of IDs currently in the list */
    int counter = returnNextIndex(processIDs);

    /* Check if any of the process IDs has finished. 
    Change their values if so. */
    for (int i = 0; i < counter; i++)
    {
        /* Grab the current pid */
        childPid = processIDs[i];

        /* If the pid is above 0, its exit status needs to be checked */
        if (childPid > 0)
        {
            childStatus = waitpid(childPid, &terminationStatus, WNOHANG);
            /* If the waitpid is not 0, it has finished and its exit status can be checked */
            if (childStatus != 0)
            {
                /* Check if the child process exited normally and return its exit status */
                if (WIFEXITED(terminationStatus))
                {
                    printf("background pid %d is done: exit value %d\n", childPid,
                           WEXITSTATUS(terminationStatus));
                    fflush(stdout);
                }
                /* Check if terminated by a signal */
                else if (WIFSIGNALED(terminationStatus))
                {
                    printf("background pid %d is done: terminated by signal %d\n", childPid,
                           WTERMSIG(terminationStatus));
                    fflush(stdout);
                }
                /* Set this pid to a negative int to exclude in later checks */
                processIDs[i] = -1;
            }
        }
    }

    return 0;
}

/* killChildProcs */
/* Receives: the list of bg process IDs 
Will iterate through any processes that have not terminated and kill them. */
int killChildProcs(int processIDs[])
{
    int count = returnNextIndex(processIDs);
    int terminationStatus;

    if (count == 0)
    {
        return 1;
    }
    /* Find any currently active processes */
    for (int i = 0; i < count; i++)
    {
        if (processIDs[i] > 0)
        {
            kill(processIDs[i], SIGTERM);
            waitpid(processIDs[i], &terminationStatus, 0);
            processIDs[i] = -1;
        }
    }

    return 0;
}

/* changeDir */
/* Receives: the userCommand struct 
/* Will change the current directory to the path indicated in the command or to HOME
if no directory path is indicated */
void changeDir(struct userCommand *currCommand)
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

    return;
}

/* processCommand */
/* Receives: the usercommand struct 
Checks if the command raises any of the 3 built in commands (cd, status, or exit)
If not, it is a child process, and is processed accordingly.
Returns: VOID */
void processCommand(struct userCommand *currCommand, int processIDs[], int exitStatus[],
                    struct sigaction SIGINT_action, struct sigaction SIGTSTP_action)
{
    char *exitFlag = "exit";
    char *cdFlag = "cd";
    char *statusFlag = "status";
    /* Check for any status flags */

    /* Check if the user wants to exit - need to complete */
    if (strcmp(currCommand->command, exitFlag) == 0)
    {
        /* Kill any child processes */
        killChildProcs(processIDs);
        freeCommand(currCommand);
        exit(0);
    }
    /* If the user wants to change directories */
    else if (strcmp(currCommand->command, cdFlag) == 0)
    {
        changeDir(currCommand);
    }
    /* Check if the user entered the status flag */
    else if (strcmp(currCommand->command, statusFlag) == 0)
    {
        /* Print out the status of the most recently executed fg command */
        if (exitStatus[0] == 0 || exitStatus[0] == 1)
        {
            printf("exit value %d\n", exitStatus[0]);
            fflush(stdout);
        }
        else
        {
            printf("terminated by signal %d\n", exitStatus[0]);
            fflush(stdout);
        }
    }
    /* Otherwise, this is not a built-in processs */
    else
    {
        createChildProcess(currCommand, processIDs, exitStatus, SIGINT_action, SIGTSTP_action);
    }
}

/* Receives input from the user. */
/* Will check for hash or newline, otherwise return the buffer */
char *getInput()
{
    char *buffer;
    size_t bufsize = 0;
    getline(&buffer, &bufsize, stdin);

    /* Check for a new line or hash symbol */
    char *newLine = "\n";
    char *hash = "#";

    /* Ignore the input and set to NULL if these characters are detected */
    if (strcmp(buffer, newLine) == 0 || buffer[0] == *hash)
    {
        buffer = NULL;
    }

    return buffer;
}

/* handle_SIGTSTP */
/* This function was constructed using the following code snippet: 
https://repl.it/@cs344/53singal2c */
void handle_SIGTSTP(int signo)
{
    if (!fgOnlyMode)
    {
        char *message = "\nEntering foreground-only mode (& is now ignored)\n: ";
        fgOnlyMode = true;
        write(STDOUT_FILENO, message, strlen(message));
        fflush(stdout);
    }
    else
    {
        char *message = "\nExiting foreground-only mode\n: ";
        fgOnlyMode = false;
        write(STDOUT_FILENO, message, strlen(message));
        fflush(stdout);
    }
}

int main()
{
    bool activateShell = true;
    int bgIDs[200];
    int fgExitStatus[1];
    fgExitStatus[0] = 0;

    /* Signal code snippet based on the following code: */
    /* https://repl.it/@cs344/53siguserc */
    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;
    SIGINT_action.sa_flags = 0;

    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    SIGTSTP_action.sa_flags = SA_RESTART;

    sigfillset(&SIGINT_action.sa_mask);
    sigfillset(&SIGTSTP_action.sa_mask);
    sigaction(SIGINT, &SIGINT_action, NULL);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    while (activateShell)
    {
        /* Check for any background processes */
        checkBackgroundProcs(bgIDs);

        char *buffer;
        /* Display the shell prompt */
        displayPrompt();
        /* Grab the command */
        buffer = getInput();
        /* tokenize the command and process it */
        if (buffer != NULL)
        {
            struct userCommand *currCommand = parseCommand(buffer);
            processCommand(currCommand, bgIDs, fgExitStatus, SIGINT_action, SIGTSTP_action);
        }

        free(buffer);
    }

    return 0;
}