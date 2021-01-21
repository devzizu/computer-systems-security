
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define VERIFICATION_LINK "http://localhost:8080/"
#define FROM    "<codeverifier.ssi2021@gmail.com>"

const int MAX_SECONDS = 30;

struct upload_status {
  int lines_read;
};

char** payload_text = NULL;

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = payload_text[upload_ctx->lines_read];
 
  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;
 
    return len;
  }
 
  return 0;
}

void perform_send_email(char* to, char* content) {

    //-----------------------------------------------------

    payload_text = malloc(sizeof(char*)*8);
    
    char buf1[100];
    sprintf(buf1, "To: %s\r\n", to);
    payload_text[0] = strdup(buf1);

    char buf2[100];
    sprintf(buf2, "From: %s (Auth-fs Team)\r\n", FROM);
    payload_text[1] = strdup(buf2);

    payload_text[2] = strdup("Subject: Auth-FileSystem - Verification Code\r\n");
    
    payload_text[3] = strdup("Content-Type: text/html; charset=\"UTF-8\"\r\n");
    payload_text[4] = strdup("Content-Transfer-Encoding: quoted-printable\r\n");
    payload_text[5] = strdup("Mime-version: 1.0\r\n");
    
    payload_text[6] = strdup(content);
    payload_text[7] = NULL;
    
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;
    upload_ctx.lines_read = 0;
 
    //-----------------------------------------------------

    curl = curl_easy_init();
  
    if(curl) {
        
        // set source email and passwd
        curl_easy_setopt(curl, CURLOPT_USERNAME, "codeverifier.ssi2021");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "SSI2021CV");

        //mail server
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
 
        // TLS/SSL
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        //curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");
        
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
        recipients = curl_slist_append(recipients, to);
        //recipients = curl_slist_append(recipients, CC);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 
        

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
        // see verbose traffic output
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
        // send message
        res = curl_easy_perform(curl);
 
        /* Check for errors */ 
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));
 
        /* Free the list of recipients */ 
        curl_slist_free_all(recipients);
 
        /* Always cleanup */ 
        curl_easy_cleanup(curl);
    }
 
    //return (int) res;
}

void send_code_validation_email(char* destEmail, char* owner, char* user, char* file, char* code) {

    // prepare payload text

    char text[500];
    
    sprintf(text, "<html><head></head><body>Hi %s!<br><br>User <i>%s</i> is trying to access <b>your file</b> (path=%s).<br>To <i>authorize</i> or <i>deny</i> this operation use the following <b>code %s</b> in the <a \nhref=3D\"%s\">verification link</a>.<br>You can just <u>ignore this e-mail</u> and code will be invalid <u>after %d seconds</u>.<br><br>Best regards,<br><b>Your auth-fs team</br></body></html>", owner, user, file, code, VERIFICATION_LINK, MAX_SECONDS);
    
    // prepare email content
    perform_send_email(destEmail, text);
}
