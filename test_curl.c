#include <stdio.h>
#include <curl/curl.h>

int main() {
    CURL *curl = curl_easy_init();
    if(curl) {
        printf("libcurl is working!\n");
        curl_easy_cleanup(curl);
        return 0;
    } else {
        printf("libcurl failed to initialize\n");
        return 1;
    }
}
