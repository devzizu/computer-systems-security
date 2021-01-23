
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

char* get_storage_path(char* wdir) {

    int i;
    for (i = strlen(wdir) - 1; wdir[i] != '/'; i--);
    wdir[i] = '\0';

    char final_path[250];

    sprintf(final_path, "%s/db/storage.db", wdir);

    return strdup(final_path);
}

char* get_certificate_path(char* wdir) {

    char final_path[250];

    sprintf(final_path, "%s/mail-cert/certificate.pem", wdir);

    printf("cert=%s\n",final_path);

    return strdup(final_path);
}
