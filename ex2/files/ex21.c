// Ranen Ivgi 208210708

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

int checkRead(int r1, int r2, int result, char c1, char c2)
{
    if (r1 < 0 || r2 < 0)
    {
        perror("Error in: read\n");
        return -1;
    }

    if (r1 == 0 && r2 == 0)
    {
        if (result == 3)
        {
            return result;
        }
        return 1;
    }

    if (r1 == 0 && (c2 != ' ' && c2 != '\n'))
    {
        return 2;
    }
    if (r2 == 0 && (c1 != ' ' && c1 != '\n'))
    {
        return 2;
    }
    return 0;
}

int compareFiles(int first, int second)
{
    int result = -1;
    char c1, c2;
    int r1, r2;
    int check;

    r1 = read(first, &c1, 1);
    r2 = read(second, &c2, 1);

    while (1)
    {
        check = checkRead(r1, r2, result, c1, c2);
        if (check)
        {
            return check;
        }

        if (c1 == c2)
        {
            r1 = read(first, &c1, 1);
            r2 = read(second, &c2, 1);
            continue;
        }

        if ((c1 == ' ' || c1 == '\n') && r1 != 0)
        {
            r1 = read(first, &c1, 1);
            result = 3;
            continue;
        }

        if ((c2 == ' ' || c2 == '\n') && r2 != 0)
        {
            r2 = read(second, &c2, 1);
            result = 3;
            continue;
        }

        if (tolower(c1) == tolower(c2))
        {
            result = 3;
            r1 = read(first, &c1, 1);
            r2 = read(second, &c2, 1);
            continue;
        }

        if (c1 != c2)
        {
            return 2;
        }

        
    }
    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        return -1;
    }

    int firstFile = open(argv[1], 'r');
    if (firstFile < 0)
    {
        perror("Error in: open");
        return -1;
    }
    int secondFile = open(argv[2], 'r');
    if (secondFile < 0)
    {
        perror("Error in: open");
        return -1;
    }

    int result = compareFiles(firstFile, secondFile);
    if (close(firstFile) < 0 || close(secondFile) < 0)
    {
        perror("Error in: close");
        return -1;
    }

    return result;
}