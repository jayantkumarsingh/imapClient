#include <stdio.h>
#include <windows.h>
#include <curl/curl.h>
#include "twilio.h"

struct MemoryStruct {
    char memory[100*1024];
    size_t size;
};  

size_t _twilio_null_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    return size * nmemb;
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size+realsize] = 0;

    return realsize;
}

int send_message_notification(char *account_sid,
                        char *auth_token,
                        char *message,
                        char *from_number,
                        char *to_number,
                        char *picture_url,
                        bool verbose)
{

        // See: https://www.twilio.com/docs/api/rest/sending-messages for
        // information on Twilio body size limits.
        if (strlen(message) > 1600) {
            fprintf(stderr, "SMS send failed.\n"
                    "Message body must be less than 1601 characters.\n"
                    "The message had %zu characters.\n", strlen(message));
            return -1;
        }

        CURL *curl;
        CURLcode res;
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();

        char url[MAX_TWILIO_MESSAGE_SIZE];
        snprintf(url,
                 sizeof(url),
                 "%s%s%s",
                 "https://api.twilio.com/2010-04-01/Accounts/",
                 account_sid,
                 "/Messages");

        char parameters[MAX_TWILIO_MESSAGE_SIZE];
        if (!picture_url) {
            snprintf(parameters,
                     sizeof(parameters),
                     "%s%s%s%s%s%s",
                     "To=",
                     to_number,
                     "&From=",
                     from_number,
                     "&Body=",
                     message);
        } else {
            snprintf(parameters,
                     sizeof(parameters),
                     "%s%s%s%s%s%s%s%s",
                     "To=",
                     to_number,
                     "&From=",
                     from_number,
                     "&Body=",
                     message,
                     "&MediaUrl=",
                     picture_url);
        }

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_NONE);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters);
        curl_easy_setopt(curl, CURLOPT_USERNAME, account_sid);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, auth_token);

        if (!verbose) {
                curl_easy_setopt(curl, 
                                 CURLOPT_WRITEFUNCTION, 
                                 _twilio_null_write);
        }

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (res != CURLE_OK) {
                if (verbose) {
                        fprintf(stderr,
                                "SMS send failed: %s.\n",
                                curl_easy_strerror(res));
                }
                return -1;
        } else if (http_code != 200 && http_code != 201) {
                if (verbose) {
                        fprintf(stderr,
                                "SMS send failed, HTTP Status Code: %ld.\n",
                                http_code);
                }
                return -1;
        } else {
                if (verbose) {
                        fprintf(stderr,
                                "SMS sent successfully!\n");
                }
                return 0;
        }

}


int main(void)
{
    CURL *curl;
    CURLcode res;
    char url[100];
    static char notification[1024];
    static struct MemoryStruct chunk;
    static struct MemoryStruct header;
    int UID=373; //email UID in the Inbox to start with

    chunk.size=0;
    header.size=0;

    curl = curl_easy_init();
    if(curl)
    {
         /* Set username and password */ 
         curl_easy_setopt(curl, CURLOPT_USERPWD, "<your emailid>@gmail.com:<your password");
         curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
         curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
         curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&header);
         curl_easy_setopt(curl, CURLOPT_VERBOSE, 0); 
         curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

         sprintf(url,"imaps://imap.gmail.com:993/INBOX/;UID=%d/;SECTION=TEXT",UID);
         curl_easy_setopt(curl, CURLOPT_URL, url);
         res = curl_easy_perform(curl);
         if((res != CURLE_OK) && (res != CURLE_REMOTE_FILE_NOT_FOUND))
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        UID=atoi(strstr(header.memory,"UIDNEXT ")+strlen("UIDNEXT "));
        UID--;

        while (1)
        {
            sprintf(url,"imaps://imap.gmail.com:993/INBOX/;UID=%d/;SECTION=TEXT",UID);
            curl_easy_setopt(curl, CURLOPT_URL, url);
             
             /* Perform the request, res will get the return code */ 
            res = curl_easy_perform(curl);

             /* Check for errors */ 
            if((res != CURLE_OK) && (res != CURLE_REMOTE_FILE_NOT_FOUND))
            {
               fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
               break;
            }
            else
             {
                if (res== CURLE_REMOTE_FILE_NOT_FOUND)
                {
                    printf ("File Not Found\n");
                    Sleep(5000); //wait for 5 secs
                    continue;
                }
                printf("%lu bytes retrieved\n", (long)chunk.size);
                if (*chunk.memory!=0)
                {
                    printf ("Body:\n %s\n",chunk.memory);
                    //next UID
                    ++UID;
                    sprintf(notification, "Parse body in chunk.memory and create sms notification");
                    printf("%s\n",notification);
                    send_message_notification("<Your twilio account sid>","<your twilio auth token", \
                        notification,"<from number>","<to number>",NULL,false);
                    chunk.size=0;
                    header.size=0;

                }
            }
     }

        /* always cleanup */ 
        printf("Exited\n");
        curl_easy_cleanup(curl);
    }
    return 0;
}