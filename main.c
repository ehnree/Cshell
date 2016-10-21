// Specs

// when you type in a command (in response to its prompt), the shell creates a child process that executes the command you entered and then prompts for more user input when it has finished.

// Your shell can be run in two ways: interactive and batch. In interactive mode, you will display a prompt (any string of your choosing) and the user will type in a command at the prompt. In batch mode, your shell is started by specifying a batch file on its command line; the batch file contains the list of commands that should be executed. In batch mode, you should not display a prompt. In batch mode you should echo each line you read from the batch file back to the user before executing it; this will help you when you debug your shells (and us when we test your programs). In both interactive and batch mode, your shell stops accepting new commands when it reads the “quit” command on a line or reaches the end of the input stream (i.e., the end of the batch file or the user types 'Ctrl-D'). The shell should then exit after all running processes have terminated.

// To exit the shell, the user can type quit or ctrl+D. This should just exit the shell and be done with it (the exit() system call will be useful here).

// Fault tolerance

// Invoked via:
	// shell [batchFile]

// commands delimited by ';'

// Error Handling - print to stderr
// An incorrect number of command line arguments to your shell program.
// The batch file does not exist or cannot be opened.
// For the following situation, you should print a message to the user (stderr) and continue processing:
// A command does not exist or cannot be executed.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define INTERACTIVE_MODE 0
#define BATCH_MODE 1

typedef struct {
	int mode;
	FILE* buffer;
	char PWD[1024];
} ENV;

typedef struct {
	void (*cd) (char*);
} CMD;

CMD cmd;
ENV env;

void shell_cd(char* path);
void read_lines();
void init_shell(int argc, char* argv[]);
void shell_fn(char* path);

int main(int argc, char* argv[])
{
	char* cwd = NULL;

	init_shell(argc, argv);
	read_lines();

	return 0;
}

void init_shell(int argc, char* argv[])
{
	env.mode = INTERACTIVE_MODE;
	env.buffer = stdin;
	if(getcwd(env.PWD, sizeof(env.PWD)) == NULL)
	{
		fprintf(stderr, "init_shell(): getcwd() error. cannot get PWD\n");
		exit(EXIT_FAILURE);
	} else {
		printf("PWD: %s\n", env.PWD);
	}

	// Set mode
	if (argc == 1) {
		// interactive mode
		env.mode = INTERACTIVE_MODE;
	} else if (argc == 2) {
		//batch mode
		env.mode = BATCH_MODE;
		//TODO what happens when fopen fails?
		env.buffer = fopen(argv[1], "r");
	} else {
		fprintf( stderr, "Proper usage of shell is: shell [batchfile]\n");
		exit(EXIT_FAILURE);
	}

	cmd.cd = &shell_cd;

}

void read_lines()
{
	char* line = NULL;
	size_t size;

	while(1) {
		// Get executable name
		if (getline(&line, &size, env.buffer) == -1) {
			printf("\n");
			exit(EXIT_SUCCESS);
			// fprintf( stderr, "Error...");
		} else {
			// Different depending on mode
			// Batch mode needs to also check for EOF.
			// Lines in format <command> <args ... > [;]

			// Check for all whitespace
			bool ws = true;
			for (int i = 0; i < size; i ++) {
				if (!isspace(line[i])) {
					ws = false;
					break;
				}
			}
			if (!ws) {
				//tokenize by (eventually arbitrary) whitespace
				//check if first argument is a command

				// strip new line char at the end
				char *pos;
				if ((pos=strchr(line, '\n')) != NULL)
				    *pos = '\0';

				shell_fn(line);
				//pass args to function
				//arg handling?
			}
		}
	}

}

void shell_fn(char* line)
{
	char* token = strtok(line, " ");

	if (strcmp(token, "cd") == 0) {
		printf("Calling cd\n");
		token = strtok(NULL, " ");


		// char buffer[strlen(token) - 1];
		// printf("%d\n", strlen(token));
		// int i = 0;
		// while (i < strlen(token) - 1) {
		// 	buffer[i] = token[i];
		// 	i++;
		// }
		// while buffer, token, strlen(1));
		// printf("directory is : %s\n", buffer);

		cmd.cd(token);
	}

}

void shell_cd(char* path){
	if(chdir(path) == -1){
		fprintf(stderr, "shell_cd(): error finding path %s\n", path);
	}
	getcwd(env.PWD, sizeof(env.PWD));
	printf("PWD is: %s\n", env.PWD);
}
