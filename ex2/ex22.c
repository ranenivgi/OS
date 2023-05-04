#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>

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
            return 0;
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

int runExecutable(int input_dir, int student_output, int errors, int results, struct dirent *student_name)
{
    pid_t pid;
    int status;
    struct timeval start_time, end_time;

    // get start time
    if (gettimeofday(&start_time, NULL) == -1)
    {
        write(2, "Error in: gettimeofday\n", strlen("Error in: gettimeofday\n"));
        return 0;
    }

    pid = fork();
    if (pid < 0)
    {
        write(2, "Error in: fork\n", strlen("Error in: fork\n"));
        return 0;
    }
    if (pid == 0)
    {
        if (lseek(input_dir, 0, SEEK_SET) == -1)
        {
            write(2, "Error in: lseek\n", strlen("Error in: lseek\n"));
            return 0;
        }
        // set the IO redirection
        if (dup2(student_output, STDOUT_FILENO) == -1 || dup2(input_dir, STDIN_FILENO) == -1 || dup2(errors, STDERR_FILENO) == -1)
        {
            write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
            return 0;
        }
        // run the program
        if (execl("compiled_file.out", "compiled_file.out", NULL) == -1)
        {
            write(2, "Error in: execl\n", strlen("Error in: execl\n"));
            return 0;
        }
    }
    else
    {
        if (waitpid(pid, &status, 0) == -1)
        {
            write(2, "Error in: waitpid\n", strlen("Error in: waitpid\n"));
            return 0;
        }

        // get end time
        if (gettimeofday(&end_time, NULL) == -1)
        {
            write(2, "Error in: gettimeofday\n", strlen("Error in: gettimeofday\n"));
            return 0;
        }

        // calculate total time
        long int elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);

        // Check if elapsed time is greater than 5 seconds and add timeout accordingly
        if (elapsed_time > 5000000)
        {
            write(results, student_name->d_name, strlen(student_name->d_name));
            write(results, ",20,TIMEOUT\n", strlen(",20,TIMEOUT\n"));
            return 0;
        }
    }
    return 1;
}

int compareOutput(char *output, int results, struct dirent *student_name)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0)
    {
        write(2, "Error in: fork\n", strlen("Error in: fork\n"));
        return 0;
    }
    if (pid == 0)
    {
        // run the first exercise to compare the output files
        if (execl("comp.out", "comp.out", "student_output.txt", output, NULL) == -1)
        {
            write(2, "Error in: execl\n", strlen("Error in: execl\n"));
            return 0;
        }
    }
    else
    {
        // check the return status and set the output file accordingly
        if (wait(&status) == -1) {
            write(2, "Error in: wait\n", strlen("Error in: wait\n"));
            return 0;
        }
        int check_status = WEXITSTATUS(status);

        if (check_status == 1)
        {
            write(results, student_name->d_name, strlen(student_name->d_name));
            write(results, ",100,EXCELLENT\n", strlen(",100,EXCELLENT\n"));
        }
        else if (check_status == 2)
        {
            write(results, student_name->d_name, strlen(student_name->d_name));
            write(results, ",50,WRONG\n", strlen(",50,WRONG\n"));
        }
        else if (check_status == 3)
        {
            write(results, student_name->d_name, strlen(student_name->d_name));
            write(results, ",75,SIMILAR\n", strlen(",75,SIMILAR\n"));
        }
    }
    return 1;
}

int rateStudent(struct dirent *student_name, char *student_folder_path, int results, int errors, int input_dir, int student_output, char *output)
{
    DIR *students_dir;
    struct dirent *file_iterator;

    if ((students_dir = opendir(student_folder_path)) == NULL)
    {
        write(2, "Error in: opendir\n", strlen("Error in: opendir\n"));
        return 0;
    }

    int c_file_exists = 0;
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

        // mark that there is c file
        c_file_exists = 1;

        // compile the .c file, move to the next student if it doesn't work
        if (!compile(compile_file_path, errors, student_name, results))
        {
            break;
        }

        if (!runExecutable(input_dir, student_output, errors, results, student_name))
        {
            break;
        }

        if (!compareOutput(output, results, student_name))
        {
            break;
        }

        // end the loop after the first c file
        break;
    }
    if (closedir(students_dir) == -1)
    {
        write(2, "Error in: closedir\n", strlen("Error in: closedir\n"));
        return 0;
    }
    return c_file_exists;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        return -1;
    }

    int cfg = open(argv[1], 'r');
    if (cfg < 0)
    {
        write(2, "Error in: open\n", strlen("Error in: open\n"));
        return -1;
    }

    char buffer[1024];
    int read_bytes = read(cfg, buffer, 1024);
    if (read_bytes == -1)
    {
        write(2, "Error in: read\n", strlen("Error in: read\n"));
        if (close(cfg) < 0)
        {
            write(2, "Error in: close\n", strlen("Error in: close\n"));
        }
        return -1;
    }

    // the first three lines from the cfg
    char *students = strtok(buffer, "\n");
    char *input = strtok(NULL, "\n");
    char *output = strtok(NULL, "\n");

    // close the cfg
    if (close(cfg) < 0)
    {
        write(2, "Error in: close\n", strlen("Error in: close\n"));
        return -1;
    }

    // make sure the students path is a folder
    if (!isFolder(students))
    {
        write(2, "Not a valid directory\n", strlen("Not a valid directory\n"));
        return -1;
    }

    // make sure the input path is a file
    if (!isFile(input))
    {
        write(2, "Input file not exist\n", strlen("Input file not exist\n"));
        return -1;
    }

    // make sure the output path is a file
    if (!isFile(output))
    {
        write(2, "Output file not exist\n", strlen("Output file not exist\n"));
        return -1;
    }

    int input_dir = open(input, 'r');
    if (input_dir == -1)
    {
        write(2, "Error in: open\n", strlen("Error in: open\n"));
        return -1;
    }

    // create results.csv file
    int results = open("results.csv", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (results == -1)
    {
        write(2, "Error in: open\n", strlen("Error in: open\n"));
        return -1;
    }

    // create error.txt file
    int errors = open("errors.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (errors == -1)
    {
        write(2, "Error in: open\n", strlen("Error in: open\n"));
        return -1;
    }

    DIR *students_dir;
    struct dirent *student_name;

    if ((students_dir = opendir(students)) == NULL)
    {
        write(2, "Error in: opendir\n", strlen("Error in: opendir\n"));
        return -1;
    }

    int c_file_exists = 0;
    // looping through the directory, printing the directory entry name
    while ((student_name = readdir(students_dir)) != NULL)
    {
        // skip over the '.' and '..' entries
        if (strcmp(student_name->d_name, ".") == 0 || strcmp(student_name->d_name, "..") == 0)
        {
            continue;
        }

        // create the output.txt file
        int student_output = open("student_output.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        if (student_output == -1)
        {
            write(2, "Error in: open\n", strlen("Error in: open\n"));
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
            if (close(student_output))
            {
                write(2, "Error in: close\n", strlen("Error in: close\n"));
            }
            continue;
        }

        // the function returns 0 if there is no .c file
        if (!rateStudent(student_name, student_folder_path, results, errors, input_dir, student_output, output))
        {
            write(results, student_name->d_name, strlen(student_name->d_name));
            write(results, ",0,NO_C_FILE\n", strlen(",0,NO_C_FILE\n"));
        }

        if (close(student_output))
        {
            write(2, "Error in: close\n", strlen("Error in: close\n"));
        }

        if (remove("student_output.txt"))
        {
            write(2, "Error in: remove\n", strlen("Error in: remove\n"));
            return -1;
        }
        remove("compiled_file.out");
    }

    // close all dirs
    if (closedir(students_dir) == -1)
    {
        write(2, "Error in: closedir\n", strlen("Error in: closedir\n"));
        return -1;
    }
    if (close(input_dir) == -1)
    {
        write(2, "Error in: close\n", strlen("Error in: close\n"));
        return -1;
    }
    if (close(results) == -1)
    {
        write(2, "Error in: close\n", strlen("Error in: close\n"));
        return -1;
    }
    if (close(errors) == -1)
    {
        write(2, "Error in: close\n", strlen("Error in: close\n"));
        return -1;
    }
    return 0;
}