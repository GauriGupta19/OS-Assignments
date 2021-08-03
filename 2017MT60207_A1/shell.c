
//  shell.c
//  Author - "Gauri Gupta"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Defining global variables
char *built_in_commands
[] = {
    "cd",
    "exit",
    "pwd",
    "history"
};
char sysPath[1024];
int num_built_in = sizeof(built_in_commands
	) / sizeof(char *);

/* This function is used to change the directory when "cd" command is 
   executed. cd ~ takes to the initial working directory*/
int my_cd(char **args)
{
    if ((args[1]==NULL) || (!(strcmp(args[1], "~") && strcmp(args[1], "~/"))))
	chdir(sysPath);
    else if (chdir(args[1]) < 0)
	printf("No such file or directory exists: \n");
    return 1;
}

/*This function is used to exit from the shell*/
int my_exit()
{
	printf("\nClosing Shell...\n");
    return 0;
}

/* This function is used to print the current working directory when "pwd" command is 
   executed */
int my_pwd()
{
    char cwd[1024];
    
    if (getcwd(cwd, sizeof(cwd)) != NULL){
        fprintf(stdout, "%s\n", cwd);
    }
    else {
        printf("getcwd() error!\n");
    }
    return 1;
}

/*retuns the maximum of two numbers*/
int maximum(int a, int b){
	int c = a>b ? a : b;
	return c;
}


/* This function prints the last 5 commands executed on the shell 
when "history" command is given */
int my_history(char **history, int count)
{
    int i;
    if(count == 1) {
        printf("History is empty!\n");
        return 1;
    }
    int max = 6 < count ? 6 : count;
    for (i = maximum(count-6, 0); i < count-1; i++) {
        printf("%s\n",history[i]);
    }
    return 1;
}

/*This functions checks if the given command matches any one of
the defined built in commands*/
int check_command(char **args)
{
    int i;
    if ( args[0] == NULL ) {
        return 0;
    }
    for (i=0; i < num_built_in; i++) {
        if (strcmp(args[0], built_in_commands
        	[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


/* This function is used to execute the defined inbuild commands.*/
int execute_command(char **args, char **history, int count)
{
    int flag;
    if (strcmp(args[0], built_in_commands
    	[0]) == 0) {
    	if(args[1]==NULL || args[2]!=NULL){
    		printf("Invalid number of arguments!\n");
    		flag = 1;
    	}
        else{
        	flag = my_cd(args);
        }
    }
    if (strcmp(args[0], built_in_commands
    	[1]) == 0) {
        flag = my_exit();
    }
    if (strcmp(args[0], built_in_commands
    	[2]) == 0) {
    	if(args[1]!=NULL || args[2]!=NULL){
    		printf("Invalid number of arguments!\n");
    		flag = 1;
    	}
    	else{
        	flag = my_pwd();
        }
    }
    if (strcmp(args[0], built_in_commands
    	[3]) == 0) {
    	if(args[1]!=NULL || args[2]!=NULL){
    		printf("Invalid number of arguments!\n");
    		flag = 1;
    	}
        else{
        	flag = my_history(history, count);
        }
    }
    return flag;
}

/* This function is used to remove a given character in the
   input string */
void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}


/*This function is used to tokenize the input line into arguments.
The white spaces in between double quotes are ignored*/
char * my_strtok(s, delim)
    char *s;            /* string to search for tokens */
    const char *delim;  /* delimiting characters */
{
    static char *lasts;
    static char *temp;
    char *check;
    *check = '"';
    int n;
    register int ch;
    static int flag =0;
    if (s == 0)
		s = lasts;
    if (*s == '"'){
		flag =1;
		s++;
    }
    do {
	if ((ch = *s++) == '\0')
	    return 0;
    } while (strchr(delim, ch));
    --s;
    if(flag ==1){
		n = strcspn(s, check);
		flag = 0;
		lasts = s + strcspn(s+n+1, delim)+n+1;

    }
	else lasts = s + strcspn(s, delim);
	if (*lasts != 0)
		*lasts++ = 0;
	removeChar(s, '"');
    return s;
  
}

/*This function is used to parse the input and store into arguments*/
char **parse_command(char *input_line)
{
    int buffer_size = 64;
    int i = 0;
    char *arg;
    char **args = malloc(buffer_size * sizeof(char*));

    arg = my_strtok(input_line, " \t\r\n\a");
    while (arg != NULL) {
        args[i] = arg;
        i++;
        arg = my_strtok(NULL, " \t\r\n\a");
    }
    args[i] = NULL;
    return args;
} 

/*This function is used to read iput from the shell*/
char *read_command(void)
{
    int bufsize = 1024;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    int i = 0;

    while ( c != '\n' ) {
        c = getchar();
        buffer[i] = c;
        i++;
    }
    
    return buffer;
}

/* This function is used to create the Shell Prompt */
void shell_prompt() {
	static char prompt[512];
	char cwd[1024];

	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		int n = strlen(sysPath);
		strcpy(prompt, "MTL458:");
		if(strncmp(sysPath, cwd, n)==0){
			strcat(prompt, "~");
			if(strlen(cwd)>n)
				strcat(prompt, cwd+n);
		}
		else strcat(prompt, cwd);
		strcat(prompt, "$ ");
		printf("%s", prompt);
	}
	else {

		perror("Error in getting curent working directory: ");
	}
	return;
}


/*Main function*/
int main(int argc, char **arg)
{
    char *input_line;
    char **command;
    int bufsize = 1024;
    char **history = malloc(sizeof(char) * bufsize);
    int child_pid;
    int count = 0;
    int flag = 1;
    int status;
    if (getcwd(sysPath, sizeof(sysPath)) != NULL) {
    }
    else {

		perror("Error in getting curent working directory: ");
	}
    while(flag)
    {
        shell_prompt();
        input_line = read_command();
        command = parse_command(input_line);
        
        if (command[0] != NULL) {
            if ( check_command(command) ) {
            	if (command[0][0] != '!') {
                    history[count] = command[0];
                    count++;
                }
                flag = execute_command(command, history, count);
            }
            else {
                history[count] = command[0];
                count++;
                child_pid = fork(); //child process is created
                // child process
                if (child_pid == 0) {
                    execvp(command[0], command);
                }
                // parent process, wait for child to finish.
                else {
                    do {
                        waitpid(child_pid, &status, WUNTRACED);
                    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                }

            }
        }
    }
    
    return 0;
}
