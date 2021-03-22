# C Shell :shell:

**C Shell** is a small shell program written in C that implements features from well known shells, such as bash. 
It is the portfolio piece for CS 344 (Operating Systems)  at Oregon State University.

**Author**: Melissa Lagunas

## Program Functionality 

The general syntax of a command is the following: 

    command [arg1 arg2 ...] [< input_file] [> output_file] [&]

The items in square brackets are optional, and the <code>&</code> is also optional.

## User Stories

The following functionality is implemented:


- [X] User is provided with a prompt <code>$:</code> for running commands
- [X] User can insert blank lines and comments, which are lines beginning with the # character. The shell will reprompt the user for a new command if these characters are inserted.
- [X] User can expand the variable $$ to retain the process id of the current command.
- [X] User can execute the commands <code>exit</code>, <code>cd</code>, and <code>status</code> via code built into the shell. <code>status</code> will return the exit status of the most recent foreground process.
- [X] User can execute other commands, which are implemented by creating new processes using a function from the <code>exec</code> family of functions
- [X] User can execute input and output redirection
- [X] User can run commands as foreground or background processes, using the symbol <code>&</code> to indicate that a command should be executed in the background.
- [X] User can use custom signal handler <code>SIGINT</code> (CTRL-C) (child foreground processes only). This process will terminate itself upon receipt of this signal. 
- [X] User can use custom signal handler <code>SIGSTP</code> (CTRL-Z) (parent processes only). 
   - If a parent process running the shell receives this signal, the shell will display an informative message to the user. The shell will then enter a state where subsequent commands can no longer be run in the background. 
   - If the user enters <code>SIGSTP</code> again, the shell will display another informative message and return back to normal condition, where the <code>&</code> operator will be honored for subsequent commands and allow for execution of background processes.


## Live Demo Video

[Check out a live demo video of the program here.](https://youtu.be/TzZW9POTWDI)

