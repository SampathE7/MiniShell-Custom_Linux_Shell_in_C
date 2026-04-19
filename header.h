#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define BUILTIN	    1
#define EXTERNAL    2
#define NO_COMMAND  3

#define BUF_SIZE     256
#define MAX_EXT_CMDS 153
#define MAX_ARGS     20

typedef struct list{
	int pid;   //to store pid of that process
	char *str; //to store command
	struct list *next; //to store address of next  node
}Slist;

extern Slist *head;
extern char* external_commands[MAX_EXT_CMDS];
extern char* builtins[];
extern char prompt_string[BUF_SIZE];
extern char input_string[BUF_SIZE];
extern int status;
extern pid_t PID;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void scan_input(char *prompt, char *input_string);
char *get_command(char *input_string);
void copy_change(char *prompt, char *input_string);
int  check_command_type(char *command);
void echo(char *input_string, int status);
void execute_internal_commands(char *input_string);
void execute_external_commands(char *input_string);
void signal_handler(int sig_num);
void extract_external_commands(void);

void insert_list(int pid, char *cmd);
void print_list(Slist *head);
void free_list(void);
void squeeze_input_string(char *input_string);
#endif
