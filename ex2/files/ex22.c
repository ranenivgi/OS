#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <dirent.h>
#include <stdio.h>

int is_file(const char *path)
{
    struct stat info;

    if (stat(path, &info) == -1)
    {
        // Error occurred while trying to get file info
        return 0;
    }
    return S_ISREG(info.st_mode);
}

int is_folder(const char *path)
{
    struct stat info;
    if (stat(path, &info) == -1)
    {
        // Error occurred while trying to get file info
        return 0;
    }
    return S_ISDIR(info.st_mode);
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
    if (!is_folder(students))
    {
        write(2, folder_err, strlen(folder_err));
        return -1;
    }

    // make sure the input path is a file
    if (!is_file(input))
    {
        write(2, input_err, strlen(input_err));
        return -1;
    }

    // make sure the output path is a file
    if (!is_file(output))
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
    struct dirent *one_student_dir;

    if ((students_dir = opendir(students)) == NULL)
    {
        write(2, opendir_err, strlen(opendir_err));
        return -1;
    }

    // looping through the directory, printing the directory entry name
    while ((one_student_dir = readdir(students_dir)) != NULL)
    {
        // skip over the '.' and '..' entries
        if (strcmp(one_student_dir->d_name, ".") == 0 || strcmp(one_student_dir->d_name, "..") == 0)
        {
            continue;
        }

        // get the student full path
        char student_folder_path[150] = "";
        strcpy(student_folder_path, students);
        strcat(student_folder_path, "/");
        strcat(student_folder_path, one_student_dir->d_name);

        // check that the student path is a folder
        if (!is_folder(student_folder_path))
        {
            continue;
        }
        
        printf("%s\n", one_student_dir->d_name);
    }
    closedir(students_dir);
    close(input);
    close(results);
    close(errors);
    return 0;
}