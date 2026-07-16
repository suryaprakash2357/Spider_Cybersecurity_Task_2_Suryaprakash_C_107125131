#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include "shell.h"
#include "network.h"

#define MAX_CMD 256
#define MAX_ARGS 64

void shell_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr, "cd: missing argument\n");
    }
    else if(chdir(args[1]) != 0){
        perror("cd");
    }
}

int parse_command(char *line, char **args){
    int i = 0;
    char *token = strtok(line, " \t\n");
    while(token != NULL && i < MAX_ARGS - 1){
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return i;
}

void execute_external(char **args){
    pid_t pid = fork();
    if (pid == 0){
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }
    else if (pid < 0) {
        perror("fork");
    }
    else{
        wait(NULL);
    }
}

void shell_loop(){
    char line[MAX_CMD];
    char *args[MAX_ARGS];
    int argc;

    while(1){
        printf("octo-shell> ");
        if (fgets(line, sizeof(line), stdin) == NULL){
            break;
        }

        argc = parse_command(line, args);
        if (argc == 0) continue;

        if (strcmp(args[0], "exit") == 0){
            printf("Exiting shell...\n");
            break;
        }
        else if (strcmp(args[0], "cd") == 0){
            shell_cd(args);
        }
        else if (strcmp(args[0], "nittalk") == 0){
            handle_nittalk(args, argc);
        }
        else{
            execute_external(args);
        }
    }
}

int main(){
    printf("Octo Shell :))\n");
    shell_loop();
    return 0;
}
