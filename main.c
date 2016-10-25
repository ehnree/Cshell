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
#include <dirent.h>
#include <fcntl.h>

#define INTERACTIVE_MODE 0
#define BATCH_MODE 1

#define NO_REDIRECT 0
#define REDIRECT_TO_NEW 1
#define REDIRECT_TO_NEW_OR_APPEND 2

#define WAIT_FOR_CHILD 0
#define RUN_IN_BACKGROUND 1


typedef struct {
	int mode;
	FILE* buffer;
	char PWD[1024];
	char** env;
	char** argv;
	int argc;
	char shell[1024];
	char** help;
} ENV;

typedef struct {
	void (*cd) (char*);
	void (*pwd)();
	void (*dir)(char*);
	void (*clr)();
	void (*pause)();
	void (*echo)(char*);
	void (*environ)();
	void (*quit)();

	// TODO
	void (*help)();
} CMD;

CMD cmd;
ENV env;

void read_lines();
void init_shell(int argc, char* argv[], char* envp[]);
void shell_fn(char* path);

void shell_echo(char* line);
void shell_cd(char* path);
void shell_pwd();
void shell_clr();
void shell_pause();
void shell_dir(char* dir);
void shell_quit();
void shell_exec(char* exec, char* line);
void shell_environ();
void shell_help();


int main(int argc, char* argv[], char* envp[])
{
	init_shell(argc, argv, envp);
	read_lines();

	return 0;
}

void init_shell(int argc, char* argv[], char* envp[])
{
	env.mode = INTERACTIVE_MODE;
	env.env = envp;
	env.argv = argv;
	env.argc = argc;
	env.buffer = stdin;

	if(getcwd(env.PWD, sizeof(env.PWD)) == NULL)
	{
		fprintf(stderr, "init_shell(): getcwd() error. cannot get PWD\n");
		exit(EXIT_FAILURE);
	} else {
		strcat(strcpy(env.shell, env.PWD), "myshell");
		env.help = realloc(env.help, (sizeof(char*) * 2));
		char* loc = (char*) malloc(sizeof(char) * 54);
		strcpy(loc, "/Users/Henry/Documents/tufts/comp111/Cshell/help.txt"); 
		*env.help = loc;

		char** temp = env.help;
		// temp++;
		temp[1] = NULL;
		// strcat(strcpy(env.help, env.PWD), "help.txt");
	}


	// Set mode
	if (argc == 1) {
		// interactive mode
		env.mode = INTERACTIVE_MODE;
	} else if (argc == 2) {
		//batch mode
		env.mode = BATCH_MODE;
		//TODO what happens when fopen fails?
		env.buffer = fopen(argv[1], "r");}
	// } else {
	// 	fprintf( stderr, "Proper usage of shell is: shell [batchfile]\n");
	// 	exit(EXIT_FAILURE);
	// }

	cmd.cd = &shell_cd;
	cmd.pwd = &shell_pwd;
	cmd.echo = &shell_echo;
	cmd.dir = &shell_dir;
	cmd.clr = &shell_clr;
	cmd.pause = &shell_pause;
	cmd.environ = &shell_environ;
	cmd.quit = &shell_quit;
	cmd.help = &shell_help;
}

void read_lines()
{
	char* line = NULL;
	char* line_seg = NULL;
	size_t size;

	while(1) {
		if(env.mode == INTERACTIVE_MODE) printf("%s ~> ", env.PWD);

		// Get executable name
		if (getline(&line, &size, env.buffer) == -1) {
			printf("\n");
			exit(EXIT_SUCCESS);
			// fprintf( stderr, "Error...");
		} else {
			// Different depending on mode
			// Batch mode needs to also check for EOF.
			// Lines in format <command> <args ... > [;]
			char *pos;
			if ((pos=strchr(line, '\n')) != NULL)
		    	*pos = '\0';

			if (env.mode == BATCH_MODE){

				char** commands = NULL;
				char** temp = NULL;
				int n_commands = 0;
				//printf("line: %s\n", line);
				line_seg = strtok(line, ";\n");
				while(line_seg != NULL){
					commands = realloc(commands, sizeof(char*) * ++n_commands);
					if(commands == NULL){
						fprintf(stderr, "failed to allocate commands");
						exit(1); //TODO what happens when we fail to allocate memory
					}

					commands[n_commands - 1] = line_seg;
					line_seg = strtok(NULL, ";");
					commands = realloc(commands, sizeof (char*) * (n_commands+1));
					//printf("seg: %s\n", line_seg);
					commands[n_commands] = 0;
				}
				temp = commands;
				while(*commands != NULL){
					shell_fn(*commands);
					commands++;
				}
				free(temp);
			} else {
			// pass stripped line
				shell_fn(line);
			}
		}
	}
}

void shell_fn(char* line)
{
	char orig_line[128];

	strcpy(orig_line, line);
	// printf("line before is: %s\n", line);
	char* token = strtok(line, " ");
	// printf("line is: %s\n", line);
	if (token != NULL) {
		if (token[0] == '.' && token[1] == '/') {
			// exec the thing
			token+=2;
			shell_exec(token, orig_line);
		} else if (strcmp(token, "cd") == 0) {
			token = strtok(NULL, " ");
			cmd.cd(token);
		} else if ((strcmp(token, "pwd")) == 0) {
			cmd.pwd();
		} else if ((strcmp(token, "echo")) == 0) {
			token = strtok(NULL, "");
			cmd.echo(token);
		} else if ((strcmp(token, "dir")) == 0) {
			token = strtok(NULL, "");
			cmd.dir(token);
		} else if ((strcmp(token, "clr")) == 0) {
			cmd.clr();
		} else if ((strcmp(token, "pause")) == 0) {
			cmd.pause();
		} else if ((strcmp(token, "environ")) == 0) {
			cmd.environ();
		} else if ((strcmp(token, "quit")) == 0) {
			cmd.quit();
		} else if ((strcmp(token, "shell")) == 0) {
			shell_exec(env.shell, orig_line);
		} else if ((strcmp(token, "help")) == 0) {
			cmd.help();
		}
	}

}

void shell_exec(char* exec, char* line)
{

	char* temp = line;
	char** res = NULL;

	// check for background execution
	int background_exec = WAIT_FOR_CHILD;
	if (strrchr(line, '&'))
		background_exec = RUN_IN_BACKGROUND;

	char* p = strtok(line, " ");
	FILE* fp = NULL;
	int redirect = NO_REDIRECT;

	// printf("exec is %s\n", exec);
	//skip to first argument
	if ( p != NULL ) {
		p = strtok(NULL, " ");
		int n_spaces = 0, i;

		while (p) {

			if (*p == '>') {
				// get output file
				if (*(p+1) == '>') {
					redirect = REDIRECT_TO_NEW_OR_APPEND;
				} else {
					redirect = REDIRECT_TO_NEW;
			  }
				p = strtok(NULL, " ");
				if (!p) {
					fprintf(stderr, "Error: No output file provided.\n");
					return;
				}
				break;
			}

			res = realloc(res, sizeof(char*) * ++n_spaces);

			if (res == NULL)
				exit(-1);

			res[n_spaces-1] = p;

			p = strtok (NULL, " ");

			res = realloc (res, sizeof (char*) * (n_spaces+1));
			res[n_spaces] = 0;
		}
	}

	pid_t pid, endPid;
	int status;
	switch ((pid = fork()))
	{
		case -1:
			// Fork failed
			perror("Parent: Error: Failed to fork\n");
			exit(EXIT_FAILURE);
			break;
		case 0:
			//child process
			if (exec == NULL) {
				printf("Parent: Error: Must provide path to a binary executable.\n");
				return;
			}

			if (redirect) {
				// open file pointer to file name
				int fd;
				if (redirect == REDIRECT_TO_NEW_OR_APPEND)
					fd = open(p, O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);
				else if (redirect == REDIRECT_TO_NEW)
					fd = open(p, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
				if (fd < 0) {
					fprintf(stderr, "Failed to write to file: %s\n", p);
					return;
				}
				dup2(fd, 1);
				close(fd);
			}
			if (strcmp(exec, "/usr/bin/less") == 0 ) {
				printf("in here\n");
				execve( exec, env.help , NULL );				
			} else {
				execve( exec, res, NULL );				
			}
			fprintf(stderr, "Failed to execute file!\n");
			break;
		default:
			// printf("Parent: Waiting on child process to complete\n");
			if (background_exec == WAIT_FOR_CHILD) {
				while ((endPid = waitpid(pid, &status, WNOHANG|WUNTRACED)) == 0) {}
				if (endPid == -1) {
					// waitpid error
					perror("Parent: Error: waitpid failed\n");
					exit(EXIT_FAILURE);
				}
			}
			if (fp)
				fclose(fp);
			free(res);
	}
}

void shell_cd(char* path)
{
	if(chdir(path) == -1){
		fprintf(stderr, "shell_cd(): error finding path %s\n", path);
	}
	getcwd(env.PWD, sizeof(env.PWD));
	printf("PWD is: %s\n", env.PWD);
}


void shell_pwd()
{
	printf("%s\n", env.PWD);
}

void shell_echo(char* line)
{
	printf("%s\n", line);
}

void shell_dir(char* dir)
{
	DIR* d;
	struct dirent* de;
	if (dir != NULL)
		d = opendir(dir);
	else
		d = opendir(".");

	if (d) {
		while ((de = readdir(d)) != NULL) {
			printf("%s\n", de->d_name);
		}
		closedir(d);
	}
}

void shell_pause()
{
	printf("waiting for enter... \n");
	while(getc(env.buffer) != '\n'){}
}
void shell_environ()
{
	char** envPtr;
	char* envVar;
	for (envPtr = env.env; *envPtr != NULL; envPtr++) {
		envVar = *envPtr;
		printf("%s\n", envVar);
	}

}
void shell_clr()
{
	printf("\033[2J");
}

void shell_quit()
{
	exit(0);
}

void shell_help()
{
	printf("%s\n", "   HNG (HNG Not GNU!) bash, version 0.0.1-release (x86_64-whatever-you-are-running-this-on)\n \
  These shell commands are defined internally.  Type [help] to see this list.\n \
  If this was a real bash, you would be able to type [help name] to find out more about the function [name].\n \
  If this was a real bash, you would be able to use [info bash] to find out more about the shell in general.\n \
  If this was a real bash, you would be able to use [man -k] or [info] to find out more about commands not in this list.\n\n \
  cd [directory]  : Change the current default directory to [directory]. If the [directory] argument is not present, report the current directory.\n \
  clr             : Clear the screen.\n \
  dir [directory] : List the contents of directory [directory].\n \
  environ         : List all the environment strings.\n \	
  echo [comment]  : Display [comment] on the display followed by a new line\n \ 
  help            : Display the user manual using the more filter.\n \					
  pause           : Pause operation of the shell until [Enter] is pressed.\n \ 							
  quit            : Quit the shell.\n \
  [NEW] sing      : Sings a sweet lullaby to the user.\n");
}
