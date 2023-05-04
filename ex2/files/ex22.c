#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/wait.h>

int isFolder(const char *path)
{
    struct stat info;
    if (stat(path, &info) == -1)
    {
        // Error occurred while trying to get file info
        return 0;
    }
    return S_ISDIR(info.st_mode);
}

int isFile(const char *path)
{
    struct stat info;

    if (stat(path, &info) == -1)
    {
        // Error occurred while trying to get file info
        return 0;
    }
    return S_ISREG(info.st_mode);
}

int compile(char *compile_file_path, int errors, struct dirent *student_name, int results)
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid == -1)
    {
        write(2, "Error in: fork\n", strlen("Error in: fork\n"));
        return 0;
    }
    // child process, compile the .c file
    else if (pid == 0)
    {
        if (dup2(errors, 1) == -1 || dup2(errors, 2) == -1)
        {
            write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
            exit(-1);
        }

        char *cmp[] = {"gcc", "-o", "compiled_file.out", compile_file_path, NULL};
        if (execvp(cmp[0], cmp) == -1)
        {
            write(2, "Error in: execvp\n", strlen("Error in: execvp\n"));
            return 0;
        }
    }
    // parent process, waiting for the compilation and writes the error if exists
    else
    {
        if (wait(&status) == -1)
        {
            write(2, "Error in: wait\n", strlen("Error in: wait\n"));
            return 0;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            write(results, student_name->d_name, strlen(student_name->d_name));
            write(results, ",10,COMPILATION_ERROR\n", strlen(",10,COMPILATION_ERROR\n"));
            return 0;
        }
    }
    return 1;
}

int rateStudent(struct dirent* student_name, char *student_folder_path, int results, int errors)
{
    char *opendir_err = "Error in: opendir\n";

    DIR *students_dir;
    struct dirent *file_iterator;

    if ((students_dir = opendir(student_folder_path)) == NULL)
    {
        write(2, opendir_err, strlen(opendir_err));
        exit(-1);
    }

    while ((file_iterator = readdir(students_dir)) != NULL)
    {
        // skip over the '.' and '..' entries
        if (strcmp(file_iterator->d_name, ".") == 0 || strcmp(file_iterator->d_name, "..") == 0)
        {
            continue;
        }

        // get the file full path
        char compile_file_path[150] = "";
        strcpy(compile_file_path, student_folder_path);
        strcat(compile_file_path, "/");
        strcat(compile_file_path, file_iterator->d_name);

        // check that the student path is a folder
        if (!isFile(compile_file_path))
        {
            continue;
        }
        // check if the file is a c file
        if (strcmp(file_iterator->d_name + strlen(file_iterator->d_name) - 2, ".c") != 0)
        {
            continue;
        }

        // compile the .c file, move to the next student if it doesn't work
        if (!compile(compile_file_path, errors, student_name, results))
        {
            break;
        }

        break;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        return -1;
    }

    char *open_err = "Error in: open\n";
    char *opendir_err = "Error in: opendir\n";
    char *close_err = "Error in: close\n";
    char *read_err = "Error in: read\n";
    char *folder_err = "Not a valid directory\n";
    char *input_err = "Input file not exist\n";
    char *output_err = "Output file not exist\n";

    int cfg = open(argv[1], 'r');
    if (cfg < 0)
    {
        write(2, open_err, strlen(open_err));
        return -1;
    }

    char buffer[1024];
    int read_bytes = read(cfg, buffer, 1024);
    if (read_bytes == -1)
    {
        write(2, read_err, strlen(read_err));
        close(cfg);
        return -1;
    }

    // the first three lines from the cfg
    char *students = strtok(buffer, "\n");
    char *input = strtok(NULL, "\n");
    char *output = strtok(NULL, "\n");

    // close the cfg
    if (close(cfg) < 0)
    {
        write(2, close_err, strlen(close_err));
        return -1;
    }

    // make sure the students path is a folder
    if (!isFolder(students))
    {
        write(2, folder_err, strlen(folder_err));
        return -1;
    }

    // make sure the input path is a file
    if (!isFile(input))
    {
        write(2, input_err, strlen(input_err));
        return -1;
    }

    // make sure the output path is a file
    if (!isFile(output))
    {
        write(2, output_err, strlen(output_err));
        return -1;
    }

    int input_dir = open(input, 'r');
    if (input_dir < 0)
    {
        write(2, open_err, strlen(open_err));
        return -1;
    }

    // create results.csv file
    int results = open("results.csv", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (results == -1)
    {
        write(2, open_err, strlen(open_err));
        return -1;
    }

    // create error.txt file
    int errors = open("errors.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (errors == -1)
    {
        write(2, open_err, strlen(open_err));
        return -1;
    }

    DIR *students_dir;
    struct dirent *student_name;

    if ((students_dir = opendir(students)) == NULL)
    {
        write(2, opendir_err, strlen(opendir_err));
        return -1;
    }

    // looping through the directory, printing the directory entry name
    while ((student_name = readdir(students_dir)) != NULL)
    {
        // skip over the '.' and '..' entries
        if (strcmp(student_name->d_name, ".") == 0 || strcmp(student_name->d_name, "..") == 0)
        {
            continue;
        }

        // get the student full path
        char student_folder_path[150] = "";
        strcpy(student_folder_path, students);
        strcat(student_folder_path, "/");
        strcat(student_folder_path, student_name->d_name);

        // check that the student path is a folder
        if (!isFolder(student_folder_path))
        {
            continue;
        }

        rateStudent(student_name, student_folder_path, results, errors);
        printf("%s\n", student_folder_path);
    }
    closedir(students_dir);
    close(input_dir);
    close(results);
    close(errors);
    return 0;
}