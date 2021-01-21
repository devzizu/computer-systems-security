
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/fs-tools.h"

static const char set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789";
static size_t set_count = sizeof(set) - 1;

char* generate_random_code(int size) {

    srand(time(NULL));

    char output[size];

    for(int i = 0; i < size; i++) {
        
        int index = (double) rand() / RAND_MAX * (set_count);
        char c = set[index];
        output[i] = c;
    }

    output[size] = '\0';

    return strdup(output);
}
