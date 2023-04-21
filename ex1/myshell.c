// Ranen Ivgi 208210708

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void split_words(char **src, char *dest)
{
    int i = 0;
    char *token = strtok(dest, " ");
    while (token != NULL)
    {
        src[i++] = token;
        token = strtok(NULL, " ");
    }
    src[i] = NULL;
}

// executes the "cd" command
int cd_command(char **src)
{
    if (strcmp(src[0], "cd") == 0)
    {
        if (chdir(src[1]) != 0)
        {
            perror("chdir error");
        }
        return 1;
    }
    return 0;
}

// executes the "history" command
int history_command(char **current_command, int history_counter, int *pid_history, char **commands_history)
{
    if (strcmp(current_command[0], "history") == 0)
    {
        for (int i = 0; i <= history_counter; i++)
        {
            printf("%d %s\n", pid_history[i], commands_history[i]);
        }
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // add the directories from the arguments to the path
    char *path = getenv("PATH");
    for (int i = 1; i < argc; i++)
    {
        strcat(path, ":");
        strcat(path, argv[i]);
    }
    setenv("PATH", path, 1);

    char *commands_history[100];
    int pid_history[100];
    char command[100] = "";
    char *current_command_split[101];
    int history_counter = 0;

    do
    {
        printf("$ ");
        fflush(stdout);
        memset(command, '\0', sizeof(command));
        scanf("%[^\n\r]", command);

        //consume the carriage and copy the command
        getchar();
        char copy_command[100];
        strcpy(copy_command, command);

        // split the command into words
        split_words(current_command_split, command);

        // check if there are only spaces entered
        if (current_command_split[0] == NULL) {
            continue;
        }

        // add the command and the pid to the history
        commands_history[history_counter] = strdup(copy_command);
        pid_history[history_counter] = getpid();
        
        // check and execute "cd" and "history" commands
        if (cd_command(current_command_split) || history_command(current_command_split, history_counter, pid_history, commands_history))
        {
            ++history_counter;
            continue;
        }

        // check for exit command
        if (strcmp(current_command_split[0], "exit") == 0)
        {
            break;
        }

        // handle the exec commands (normal commands)
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork failed");
            break;
        }
        else if (pid == 0)
        {
            if (execvp(current_command_split[0], current_command_split) == -1)
            {
                perror("execvp failed");
            }
            break;
        }
        else
        {
            // set the pid of the child
            pid_history[history_counter] = pid;
            wait(NULL);
        }
        ++history_counter;
    } while (1);
}
