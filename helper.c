#include "header.h"

/*
 * Function: scan_input
 *  - Prints prompt
 *  - Reads input
 *  - Identifies command type
 *  - Executes builtin or external commands
 */
void scan_input(char *prompt, char *input_string)
{
	//registering the signal handlers 
	signal(SIGINT, signal_handler);
	signal(SIGTSTP, signal_handler);

	while(1)
	{
		//printing the prompt string
		printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET, prompt);
	
		/*__fpurge(stdin);
		scanf("%[^\n]", input_string);
		getchar();*/

		//for reading the input safely
		if(fgets(input_string, BUF_SIZE, stdin) == NULL){
			clearerr(stdin);
			continue;
		}

		input_string[strcspn(input_string, "\n")]='\0'; //removes the new line
		if(input_string[0] == '\0')  //if it is a empty command
			continue;
		
		//PS1 handling
		if(strncmp(input_string, "PS1=", 4)==0){
			copy_change(prompt, input_string); 
			continue;
		}
		if(input_string[0] == '\0')        
			continue;

		char* command = get_command(input_string);  //extracting the command(first word)
		
		//if command is "clear"
        	if(strcmp(command, "clear")==0){
                	system("clear");
			continue;
        	}
		
		int cmd_type = check_command_type(command); //storing the command type

		if(cmd_type == BUILTIN)
			execute_internal_commands(input_string);  //builtins should run in parent
		else if(cmd_type == EXTERNAL){	//external commands should be executed in child process
			PID = fork();  
			if(PID > 0){	//parent process
				waitpid(PID, &status, WUNTRACED);
				PID=-1;
			}
			else if(PID == 0){  //child process
				
				//register signals to default behaviour
				signal(SIGINT, SIG_DFL);
        			signal(SIGTSTP, SIG_DFL);
				
				execute_external_commands(input_string); 
				exit(1); 
			}
			else{
				perror("fork");
			}
		}
		else
			printf(ANSI_COLOR_RED "Error : No such command found"ANSI_COLOR_RESET "\n");
	}
}

/* Function: copy_change
 * Changes shell prompt using PS1= syntax*/
void copy_change(char *prompt, char *input_string)
{
	//spaces are not allowed
	if(strchr(input_string, ' ') != NULL){
		printf(ANSI_COLOR_RED "Error: Usage PS1=NEW_PROMPT\n" ANSI_COLOR_RESET);
		return;
	}
	strcpy(prompt,input_string+4);
	strcat(prompt,":");
}

/* Function: get_command
 * Extracts first word (command name) from input */
char* get_command(char* command)
{
	static char buf[BUF_SIZE];
	
	//for skipping leading spaces
	int i=0;
	while(command[i]==' ' || command[i]=='\t' || command[i]=='\n')
		i++;

	int j=i;
	for(; command[j]!='\0'; j++){
		if(command[j]==' ' || command[j]=='\n' || command[j]=='\t')
			break;
	}

	strncpy(buf, command+i, j-i);
	buf[j-i]='\0';

	return buf;
}

/* Function: check_command_type
 * Checks whether command is:
 *  - BUILTIN
 *  - EXTERNAL
 *  - NO_COMMAND
 */
int check_command_type(char *command)
{
	int i=0;
	while(builtins[i]){
		if(strcmp(builtins[i], command)==0)
			return BUILTIN;
		i++;
	}

	i=0;
	while(external_commands[i]){
		if(strcmp(external_commands[i], command)==0)
			return EXTERNAL;
		i++;
	}
	return NO_COMMAND;
}

/* Function: extract_external_commands
 * Reads external commands from a file (external_command.txt) and stores them into the global array external_commands[].*/
void extract_external_commands()
{
	FILE *fp = fopen("external_command.txt","r");
	if(fp == NULL){
		perror("fopen");
		return;
	}

	char buffer[BUF_SIZE];
	int i=0;

	//reading file line by line
	while(fgets(buffer, BUF_SIZE, fp)!=NULL && i<MAX_EXT_CMDS)
	{
		buffer[strcspn(buffer, "\n")] = '\0'; //remove trailing newline
	
		external_commands[i] = malloc(strlen(buffer)+1); //allocating exact memory	
		if(external_commands[i] == NULL){
			perror("malloc");
			for(int j=0; j<i; j++)
				free(external_commands[j]);
			fclose(fp);
			return;
		}

		strcpy(external_commands[i], buffer);
		i++;
	}
	
	external_commands[i] = NULL;
	fclose(fp);
}


/* Function: execute_internal_commands
 * Executes commands that are built-in. */
void execute_internal_commands(char* input_string)
{
	//if command is "pwd"
	if(strncmp(input_string, "pwd",3) == 0){
		char buffer[BUF_SIZE];
		getcwd(buffer, sizeof(buffer));
		printf(ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "\n", buffer);
	}
	
	//if command is "cd" 
	else if(strncmp(input_string, "cd",2) == 0){
		squeeze_input_string(input_string);
		
		char *path = input_string+2;
		while(*path == ' ')
			path++;
		if(*path == '\0')
			printf("\n");
		if(chdir(path) == -1)
			printf(ANSI_COLOR_RED "Error: Invalid path" ANSI_COLOR_RESET "\n");
	}

	//if command is "exit"
	else if(strcmp(input_string, "exit")==0){
		free_list();
		for(int i=0; external_commands[i]; i++)
			free(external_commands[i]);
		exit(0);
	}


	//if command is "echo"
	else if(strncmp(input_string, "echo", 4)==0)
	{
		char *arg = input_string + 4;
		while(*arg == ' ')
			arg++;

		if(strcmp(arg, "$$")==0)
			printf("%d\n",getpid());  //printing the shell id

		else if(strcmp(arg, "$?")==0){
			printf("%d\n",WEXITSTATUS(status));	
		}
		else if(strcmp(arg, "$SHELL")==0){
			char *path = getenv("SHELL");
			if(path)
				printf(ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "\n", path);
		}
		else
			return;
	}

	//if cmd is "jobs"
	else if(strcmp(input_string,"jobs") == 0){
    		if(head == NULL) 
    			printf(ANSI_COLOR_RED "No jobs found" ANSI_COLOR_RESET "\n");
         	else 
            		print_list(head);
        
        	return;
    	}

	//if command is "fg"
	else if(strncmp(input_string,"fg",2) == 0)
	{
		if(!head){
			printf(ANSI_COLOR_RED "-bash: fg: no such jobs" ANSI_COLOR_RESET "\n");
         	   	return;
        	}	

		//take job from beginning
		Slist *temp = head;
		head = head->next;

		printf("Foregrounding PID is   %d:   %s\n", temp->pid, temp->str);

		PID = temp->pid;  //mark as foreground 
		kill(temp->pid, SIGCONT); //resumes the temp(job)
	
		waitpid(temp->pid, &status, WUNTRACED);

		//if stopped again, re-insert
		if(WIFSTOPPED(status)){
			insert_list(temp->pid, temp->str);
		}

		else{
			free(temp->str);
			free(temp);
		}

		PID=-1;
		return;
	}

	//if it is "bg"
	else if(strcmp(input_string, "bg") == 0){

		signal(SIGCHLD, signal_handler);
		if(head == NULL){
			printf(ANSI_COLOR_RED "-bash: bg: no such jobs" ANSI_COLOR_RESET "\n");
			return;
		}

		Slist *temp = head;
		head = head->next;
		
		printf("Running background PID   %d:   %s\n", temp->pid, temp->str);

		kill(temp->pid, SIGCONT);  //resumes execution

		free(temp->str);
		free(temp);
		return;
	}

}

/*
 * Function: execute_external_commands
 * Executes external (non-builtin) commands.
 * Supports single command and piped commands.
 *
 * Example:
 *      ls
 *      ls | grep txt | wc -l
 *
 * This function is executed ONLY by the CHILD process.
 */
void execute_external_commands(char* input_string)
{
	if(input_string[0] == '\0'){
		printf("Error: \nUsage: <command1> '|' <command2> '|' <command3>\n");
		exit(1);
	}

	char buffer[BUF_SIZE];
	char* cmds[15][MAX_ARGS];  //max we can use 15 pipe commands
	int cmd_count=0; //no.of commands separated by '|'
       	int arg_count=0; //no.of arguments for current command

	strcpy(buffer, input_string);
	
	char* token = strtok(buffer, " \t");  //tokenize the input using strtok
	
	//pipe cannot be at first
	if(token != NULL && strcmp(token, "|")==0){
		printf(ANSI_COLOR_RED "Error: \nUsage: <command1> '|' <command2> '|' <command3>" ANSI_COLOR_RESET "\n");
		exit(1);
	}
	
	while(token != NULL)
	{
		if(strcmp(token, "|")==0){  //if pipe found
					    
			if(arg_count == 0){  //consecutive pipes or empty command
				printf(ANSI_COLOR_RED "Error: \nUsage: <command1> '|' <command2> '|' <command3>" ANSI_COLOR_RESET "\n");
				exit(1);
			}
			cmds[cmd_count][arg_count] = NULL;
			cmd_count++;
			arg_count=0;

			if(arg_count >= MAX_ARGS){
				printf(ANSI_COLOR_RED "Error: too many pipe commands" ANSI_COLOR_RESET "\n");
				exit(1);
			}
		}
		
		else{
			if(arg_count > MAX_ARGS-1){
				printf(ANSI_COLOR_RED "Error: too many arguments" ANSI_COLOR_RESET "\n");
				exit(1);
			}
			cmds[cmd_count][arg_count++] = token;
		}

		token = strtok(NULL, " \t");
	}
	
	if(arg_count==0){ //for trailing pipes
		printf(ANSI_COLOR_RED "Error: \nUsage: <command1> '|' <command2> '|' <command3>" ANSI_COLOR_RESET "\n");
		exit(1);
	}

	cmds[cmd_count][arg_count] = NULL;
	cmd_count++;

	//if NO PIPES were present
	if(cmd_count == 1){
		execvp(cmds[0][0], cmds[0]);
		perror("execvp");
		exit(1);
	}

	//if PIPE EXIST
	int pipefd[2];
	int fd=0;

	for(int i=0; i<cmd_count; i++)
	{
		if(pipe(pipefd) == -1){
			perror("pipe");
			exit(1);
		}

		PID = fork();
		if(PID > 0)	//parent process
		{	
			while( wait(NULL) > 0);
			
			close(pipefd[1]);
			if(fd != 0)
				close(fd);
			fd = pipefd[0];
		}

		else if(PID==0)
		{
			if(fd != 0){
				dup2(fd, 0);
				close(fd);
			}

			//writing to next PIPE
			if(i != (cmd_count - 1))
				dup2(pipefd[1], 1);

			close(pipefd[0]);
			close(pipefd[1]);

			execvp(cmds[i][0], cmds[i]);
			perror("execvp");
			exit(1);
		}
		
		else{
			perror("fork");
			exit(1);
		}
	}
	
	if(fd != 0)
		close(fd);
	exit(0);
}

/* Function: signal_handler
 * Handles Ctrl+C, Ctrl+Z, SIGCHLD */
void signal_handler(int signum)
{
	static int count = 1;
	if(signum == SIGINT)
	{
		if(PID > 0)
			printf("\n");
		if(PID == -1){
			printf(ANSI_COLOR_GREEN "\n%s " ANSI_COLOR_RESET, prompt_string);
			fflush(stdout);
		}
	}

	else if(signum == SIGTSTP)
	{
		if(PID > 0){
			kill(PID, SIGTSTP);
			insert_list(PID, input_string);
			printf("\n[%d]+  Stopped   %d   %s\n", count++,PID,input_string);
			PID = -1;
		}
		else{
			printf(ANSI_COLOR_GREEN "\n%s " ANSI_COLOR_RESET, prompt_string);
			fflush(stdout);
		}
	}

	else if(signum == SIGCHLD){
		while(waitpid(-1, &status, WNOHANG) > 0);  //to cleanup the background process
	}
}

/* Function: insert_list
 * inserts the stopped process into jobs*/
void insert_list(int pid, char *cmd)
{
	Slist *new = malloc(sizeof(Slist));
	if(new == NULL){
		perror("malloc");
		return;
	}

	new->str = malloc(strlen(cmd) + 1);
	if(new->str == NULL){
		perror("malloc");
		free(new);
		return;
	}	
	
	new->pid = pid;
	strcpy(new->str, cmd);

	new->next = head; //inserting at beginning
	head = new;
}

/* Function: print_list
 * prints the data from last using recursion*/
void print_list(Slist *temp)
{
	static int i=1;
	if(temp == NULL){
		return;
	}
	
	print_list(temp->next);
	printf("[%d]+ Stopped   %d   %s\n",i++, temp->pid, temp->str);
}

/* Function: free_list
 * it frees the allocated memory for jobs*/
void free_list()
{
	Slist *temp = head;
	while(head){
		temp = head;
		head = head->next;
		free(temp->str);
		free(temp);
	}
}

/* Function: squeeze_input_string
 * It squeezes the input*/
void squeeze_input_string(char* str)
{
	int read=0, write=0;
	int in_space=0;

	while(str[read] != '\0')
	{
		if(str[read] == ' ' || str[read] == '\t')
		{
			if(!in_space){
				str[write++] = ' ';
				in_space = 1;
			}
		}

		else{
			str[write++] = str[read];
			in_space = 1;
		}
		read++;
	}
	str[write] = '\0';
}
