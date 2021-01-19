#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
 
/* This is a simple example showing how to send mail using libcurl's SMTP
 * capabilities. It builds on the smtp-mail.c example to add authentication
 * and, more importantly, transport security to protect the authentication
 * details from being snooped.
 *
 * Note that this example requires libcurl 7.20.0 or above.
 */ 
 
#define FROM    "<codeverifier.ssi2021@gmail.com>"
#define TO      "<a85729@alunos.uminho.pt>"
//#define CC      "<a85227@alunos.uminho.pt>"

// Mensagem exemplo
/*
static const char *payload_text[] = {
  "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
  "To: " TO "\r\n",
  "From: " FROM " (User Test)\r\n",
  //"Cc: " CC " (Another example User)\r\n",
  "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
  "rfcpedant.example.org>\r\n",
  "Subject: SMTP TLS example message\r\n",
  "\r\n", /* empty line to divide headers from body, see RFC5322  
  "The body of the message starts here.\r\n",
  "\r\n",
  "It could be a lot of lines, could be MIME encoded, whatever.\r\n",
  "Check RFC5322.\r\n",
  NULL
};
*/

static const char *payload_text[] = {
  "To: " TO "\r\n",
  "From: " FROM " (User Test)\r\n",
  "Subject: Código de Verificação\r\n",
  "Insere o teu codigo de verfificacao, para validar o pedido",
  NULL
};

struct upload_status {
  int lines_read;
};
 
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
 
int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;
 
  upload_ctx.lines_read = 0;
 
  curl = curl_easy_init();

  if(curl) {
    /* Colocar o username e a password */ 
    curl_easy_setopt(curl, CURLOPT_USERNAME, "codeverifier.ssi2021");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "SSI2021CV");
 
    /* O link em que irá ser trabalhado o mail */ 
    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
 
    /* Use TLS/SSL*/ 
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  
    /* Evitar a verificação do certerficado, desabilitando parte da segurança da camada de transporte */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");

    /* Note that this option isn't strictly required, omitting it will result
     * in libcurl sending the MAIL FROM command with empty sender data. All
     * autoresponses should have an empty reverse-path, and should be directed
     * to the address in the reverse-path which triggered them. Otherwise,
     * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
     * details.
     */ 
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
 
    /* Add two recipients, in this particular case they correspond to the
     * To: and Cc: addressees in the header, but they could be any kind of
     * recipient. */ 
    recipients = curl_slist_append(recipients, TO);
    //recipients = curl_slist_append(recipients, CC);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 
    /* We're using a callback function to specify the payload (the headers and
     * body of the message). You could just use the CURLOPT_READDATA option to
     * specify a FILE pointer to read from. */ 
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    /* Ver um registo de tráfico/debugging */ 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
    /* Envia a mensagem criada */ 
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
 
  return (int)res;
}
