#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 500
#define MAX_ARGS_LENGTH 50

void execute_command(char *command) {

    char *cmd[MAX_ARGS_LENGTH];
    char *token = strtok(command, "|");
    int i = 0;
    while (token != NULL) {
        cmd[i++] = token;
        token = strtok(NULL, "|");
    }
    cmd[i] = NULL;

    int num_commands = i;

    int pipes[num_commands - 1][2];
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Ошибка при создании канала");
            return;
        }
    }

    pid_t pid;
    for (int i = 0; i < num_commands; i++) {
        pid = fork();
        if (pid == 0) {
            if (i != 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i != num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }


	    if (strchr(cmd[i], '>') == NULL) {
		    char *args[MAX_ARGS_LENGTH];
		    token = strtok(cmd[i], " ");
		    int j = 0;
		    while (token != NULL) {
		        args[j++] = token;
		        token = strtok(NULL, " ");
		    }
		    args[j] = NULL;

		    execvp(args[0], args);
		    perror("Ошибка выполнения команды");
		    exit(EXIT_FAILURE);
	    }
	    else {
		    char *args[MAX_ARGS_LENGTH];
		    char *tok = strtok(cmd[i], ">");
		    int i = 0;
		    while (tok != NULL) {
			args[i++] = tok;
			tok = strtok(NULL, ">");
		    }
		    args[i] = NULL;

		    char *filename = args[1];
		    while (*filename == ' ') {
			filename++;
		    }
		    int len = strlen(filename);
		    while (len > 0 && filename[len - 1] == ' ') {
			filename[len - 1] = '\0';
			len--;
		    }
		    
		freopen(filename, "w", stdout);
		
		char *command_args[MAX_ARGS_LENGTH];
		token = strtok(args[0], " ");
		int j = 0;
		while (token != NULL) {
		    command_args[j++] = token;
		    token = strtok(NULL, " ");
		}
		command_args[j] = NULL;

		execvp(command_args[0], command_args);
		perror("Ошибка выполнения команды");
		exit(EXIT_FAILURE);
		    
	    }
	              
        } else if (pid < 0) {
            perror("Ошибка при создании нового процесса");
        }
    }

    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    while (1) {
        printf("Введите команду (или 'exit' для выхода): ");
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        
        command[strcspn(command, "\n")] = 0;
        if (strcmp(command, "exit") == 0) {
            break;
        }

        if (strncmp(command, "cd ", 3) == 0) {
            char *path = command + 3;
            if (chdir(path) != 0) {
                perror("Ошибка при изменении каталога");
            }
        } else {
            execute_command(command);
        }
    }
    return 0;
}
