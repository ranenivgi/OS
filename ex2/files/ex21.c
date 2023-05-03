// Ranen Ivgi 208210708

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

int checkRead(int r1, int r2, int result)
{
    if (r1 < 0 || r2 < 0)
    {
        printf("Error in: read\n");
    }

    if (r1 == 0 && r2 == 0)
    {
        if (result == 3)
        {
            return result;
        }
        return 1;
    }

    if (r1 == 0 || r2 == 0)
    {
        return 2;
    }
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
        check = checkRead(r1, r2, result);
        if (check > 0)
        {
            return check;
        }

        if (c1 == c2)
        {
            continue;
        }

        if (c1 == ' ' || c1 == '\n')
        {
            r1 = read(first, &c1, 1);
            check = checkRead(r1, r2, result);
            if (check > 0)
            {
                return check;
            }
            result = 3;
            continue;
        }

        if (c2 == ' ' || c2 == '\n')
        {
            r2 = read(second, &c2, 1);
            check = checkRead(r1, r2, result);
            if (check > 0)
            {
                return check;
            }
            result = 3;
            continue;
        }

        if (tolower(c1) == tolower(c2))
        {
            result = 3;
            continue;
        }

        r1 = read(first, &c1, 1);
        r2 = read(second, &c2, 1);
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
        return -1;
    }
    int secondFile = open(argv[2], 'r');
    if (secondFile < 0)
    {
        return -1;
    }

    close(firstFile);
    close(secondFile);
    return compareFiles(firstFile, secondFile);
}