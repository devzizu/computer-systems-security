
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <json-c/json.h>

#include "include/auth-api.h"

char* process_get_code_confirmed(int code);


int main(int argc, char* argv) {
    
    // user calls open() and owner gets contacted
    
    printf("open() called...\n");

    struct timespec t = { 1/*seconds*/, 0/*nanoseconds*/};
        
    int sec = 0; 
    int codeConfirmed = -1;

    while (sec < 3){
    
        printf("%d seconds passed...\n", sec+1);
        nanosleep(&t,NULL);
        fflush(stdout); //see below
    
        //call api
        char* res = process_get_code_confirmed(1);
        
        if (res) {
            printf("call api result = %s\n", res);
            if (!strcmp("authorize", res)) {
                codeConfirmed = 1;
                break;
            } else if (!strcmp("deny", res)){
                codeConfirmed = 0;
                break;
            }
        }
            
        sec++;
    }

    if (codeConfirmed == 1) {
        printf("OK access granted...\n");
    } else if (codeConfirmed == 0) {
        printf("Access denied by owner...\n");
    } else {
        printf("Time expired, access not granted...\n");
    }

    return 0;
}

char* process_get_code_confirmed(int code) {
    
    char *res = NULL;
    
    char* get_result = curl_get_code_confirmed(code);
 
    // got response
    if (get_result) {
            
        struct json_object *parsed_json;
        struct json_object *result_field;

        parsed_json = json_tokener_parse(get_result);

        //{"result":"Not Found"}
        json_object_object_get_ex(parsed_json, "result", &result_field);
        res = strdup(json_object_get_string(result_field));
    }

    return res;
}
