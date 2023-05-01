// Ranen Ivgi 208210708

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int compareFiles (char* first, char* second) {

}

int main(int argc, char *argv[]) {
    char *firstFile = argv[1];
    char *secondFile = argv[2];

    return compareFiles(firstFile, secondFile);
}