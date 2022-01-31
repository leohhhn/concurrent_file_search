#include "helpers/helpers.h"

int main(int argc, char *argv[]) {
    printf("Path relative to the working directory is: %s\n", argv[0]);

    scanFile("data/test.txt");
    return 0;
}