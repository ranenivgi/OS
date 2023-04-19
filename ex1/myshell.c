// Tom Ben-Dor
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // add the directories from the arguments to the path
    char *path = getenv("PATH");
    for (int i = 1; i < argc; i++)
    {
        strcat(path, ":");
        strcat(path,argv[i]);
    }
    setenv("PATH", path, 1);

    char* commands_history[100];
    int pid_history[100];
    char command[100] = "";
    char* current_command_split[100];
    int i = 0;
    int history_counter = 0;


    
    do {
        printf("$ ");
        fflush(stdout);
        scanf(" %[^\n]", command);
        
        //add the command to the history
        commands_history[history_counter] = strdup(command);

        //split the command into words
        char *token = strtok(command, " ");
        while (token != NULL) {
            current_command_split[i++] = token;
            token = strtok(NULL, " ");
        }

        if (strcmp(current_command_split[0], "cd") == 0) {
            pid_history[history_counter] = getpid();
            if (chdir(current_command_split[1]) != 0) {
                perror("chdir() error");
                return 1;
            }
        }

        if (strcmp(current_command_split[0], "history") == 0) {
            pid_history[history_counter] = getpid();
            for (int i = 0; i < history_counter + 1; i++) {
                printf("%d %s\n", pid_history[i], commands_history[i]);
            }
        }
        history_counter++;
    } while (strcmp(command, "exit") != 0);

}