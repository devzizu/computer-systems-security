
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "include/auth-api.h"

//POST / ...{ code: ..., decision: ... }
#define ROUTE_CONFIRM "http://localhost:3000/confirm"
//GET / ...code?id=X
#define ROUTE_GETCODE "http://localhost:3000/code"

//https://curl.se/libcurl/c/simple.html

struct string {
    
    char* ptr;
    size_t len;
};

size_t create_string_func(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

char* curl_get_code_confirmed(int code) {
    
    CURL *curl;    
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        
        struct string json_response;
        init_string(&json_response);

        char request_url[150];
        sprintf(request_url, "%s?id=%d", ROUTE_GETCODE, code);
        curl_easy_setopt(curl, CURLOPT_URL, request_url); 
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, create_string_func);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
            
            curl_easy_cleanup(curl);
            
            return NULL;
        }

        curl_easy_cleanup(curl);
            
        return strdup(json_response.ptr);
    }

    return NULL;
}
