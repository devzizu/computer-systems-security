
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <json-c/json.h>

#include "include/processor-thread.h"
#include "include/auth-api.h"

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
