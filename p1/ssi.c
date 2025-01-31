#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


#define PATH_LENGTH 1000
#define HOST_NAME_MAX 1000
#define FORMATTED_PROMPT_LENGTH 1000
#define ARGUMENT_LIMIT 1000


void change_directory(char* path) {

	if (chdir(path)== -1) { //if chdir fails then prit error message

		perror("cd");
	}
}



int main()
{

	char *argv[ARGUMENT_LIMIT];
	pid_t pid;
	int status;


	while (1) {

		//get current working dir, username and hostname
		char cwd[PATH_LENGTH];
		getcwd(cwd,sizeof(cwd));
		char *username;
		username = getlogin();
		char hostname[HOST_NAME_MAX];
		gethostname(hostname,sizeof(hostname));

		//format shell prompt
		size_t promptSize = snprintf(NULL,0,"%s@%s: %s > ",username,hostname,cwd)+1;
		char *prompt = malloc(promptSize);
		snprintf(prompt,promptSize,"%s@%s: %s > ",username,hostname,cwd);

		//display shell prompt, should be updated
		char* input = readline(prompt);

		//if user types exit then leave the loop
		if (input && strcmp(input,"exit")==0) {
			free(input);
			free(prompt);
			break;
		}


		//separate command input by space
		int user_input_arg = 0;
		argv[user_input_arg] = strtok(input," ");
		while (argv[user_input_arg]!=NULL){
			//printf("%s\n",argv[i]);
			user_input_arg++;
			argv[user_input_arg] = strtok(NULL," ");

		}


		//check if user command is cd, if it is then give change directory the path argument

		if(strcmp(argv[0],"cd")==0) {

			if ((argv[1]==NULL) || (strcmp(argv[1],"~")==0) ){

				char* home;
				home = getenv("HOME");
				change_directory(home);
			} else {

				change_directory(argv[1]);

			}

		}else{


			//clone this process by fork, creates an indentical process
			pid = fork();


			if (pid == 0) { //we are in the child process and want to execute the required command

				execvp(argv[0],argv);
				//execvp should not return here, if it does then break out
				perror("execvp");
				break;

			}else if (pid < 0) { //the fork did not work

				perror("fork");

			}else{ //we are in the parent process and need to wait until the child process is finished whatever command it is running

				waitpid(pid, &status, 0);

			}

			free(input);
			free(prompt);

		}
	}

	return 0;
}


