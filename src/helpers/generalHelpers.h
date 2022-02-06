#include <math.h>
#include <string.h>


/// Returns 1 if number is prime, 0 otherwise
int isNumberPrime(int input) {
    if (input <= 1) return 0;
    for (int i = 2; i <= sqrt(input); i++)
        if (input % i == 0) return 0;
    // printf("%d\n", input);
    return 1;
}

int parseCommand(char *command) {

}







