/* 
 * PROJECT: MiniShell
 * DESCRIPTION:  A simple shell implementation supporting built-in commands, external commands, job control, signal handling and pipes.
 
 * The Mini Shell project is a simplified implementation of a Unix/Linux command-line shell developed using the C programming language and Linux system calls.
 * This project aims to help students understand how a shell works internally, including process creation, command execution, signal handling, and job control.
 * The shell provides an interactive prompt where the user can enter commands. These commands can be either built-in commands (handled by the shell itself) or external 
   commands (executed using system calls like fork() and execvp()).

 * The main objectives of this project are:
	-To understand how a Linux shell works internally
	-To learn process management using fork(), exec(), and wait()
	-To implement built-in commands such as cd, pwd, echo, exit
	-To handle external commands using execvp()
	-To implement signal handling for Ctrl+C and Ctrl+Z
	-To support job control using jobs, fg, and bg
	-To gain hands-on experience with Linux system programming */

#include "header.h"

int status; //stores exit status of last command
pid_t PID=-1; //PID od a process

char prompt_string[BUF_SIZE] = "minishell:"; //default prompt
char input_string[BUF_SIZE];  

//list of built-in commands
char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
                        "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
                        "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", "fg","bg", "jobs", NULL};

char* external_commands[MAX_EXT_CMDS];  //for external commands

Slist *head = NULL; //initially head pointer points to NULL address

int main()
{
	system("clear");  //clears the terminal screen at beginnig

	extract_external_commands(); //extracting the external commands from .txt file
	
	scan_input(prompt_string, input_string); //the actual execution starts from here
	
	return 0;
}
