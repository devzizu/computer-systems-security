
#include <string.h>

#include "include/mail.h"

int main() {

    char* email = strdup("jazevedo960@gmail.com");
    char* owner = strdup("devzizu");
    char* user  = strdup("joseph");
    char* file  = strdup("file");
    char* code  = strdup("123dsafa1");

    send_code_validation_email(email, owner, user, file, code);

    return 0;
}
