# C Shell :shell:

**C Shell** is a small shell program written in C that implements features from well known shells, such as bash. 
It is the portfolio piece for CS 344, Operating Systems  at Oregon State University.

## Program Functionality 

The general syntax of a command line is the following: 

    command [arg1 arg2 ...] [< input_file] [> output_file] [&]

The items in square brackets are optional, and the & is also optional, but will indicate that the command needs to be executed in the background.

## User Stories

The following functionality is implemented:


- [X] User is provided with a prompt $: for running commands
- [X] User can insert blank lines and comments, which are lines beginning with the # character, and the shell program will ignore them.
- [X] User can use the variable $$ to retain the pid number for the process created by the command.
- [X] User can execute the commands exit, cd, and status via code built into the shell.
- [X] User can execute other commands, which are implemented by creating new processes using a function from the exec family of functions
- [X] User can execute input and output redirection
- [X] User can run commands as foreground or background processes
- [X] User can use custom signal handlers SIGINT (CTRL-C) (child processes only) and SIGTSTP (CTRL-Z) (parent processes only)


## Live Demo Video

[Check out a live demo video of the program here.](https://youtu.be/TzZW9POTWDI)

