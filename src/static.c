#include "static.h"


char* get_file(int file_fd, struct stat statbuf, char* address){

    int fileLen = statbuf.st_size;
    char* filename = basename(address);
    char header[1000];
    sprintf(header, 
    "HTTP/1.1 200 OK\n"
    "Content-Type: application/octet-stream\n"
    "Accept-Ranges: bytes\n"
    "Content-Disposition: attachment; filename=\"%s\" \n" 
    "Connection: close\n"
            "\n",filename);
    char *response = (char*) malloc(fileLen + strlen(header));
    strncpy(response, header, strlen(header));
    read(file_fd, response+strlen(header), fileLen);
    return response;
}

char* get_directory(int dir_fd, char* address){
    char header[300];
    sprintf(header, 
        "HTTP/1.1 200 OK \n"
        "Content-Type: text/html \n"
        "\n");
    char *response = (char*) malloc(strlen(header) + 1000);
    strncpy(response, header, strlen(header));
    sprintf(response, "<html> \n"
                      "<head><title>Index of %s</title></head>\n"
                    "<body> \n"
                    "<h1>Index of %s</h1><hr><pre><a href=\"../\">../</a> \n", address, address);

    int size = strlen(header) + 1000 - sizeof(response);

    struct dirent *entry;
    while ((entry = readdir(address)) != NULL) {
        char *html_entry = (char*) malloc(1000);
        char filePath[100];
        snprintf(filePath, sizeof(filePath), "%s/%s", address, entry->d_name);
        struct stat fileStat;
        if (stat(filePath, &fileStat) == 0) {
            char timeStr[20];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_mtime));
            if (S_ISREG(fileStat.st_mode)) {
                sprintf(html_entry, "<a href=\"%s\">%s</a>                                            %s                   %lld \n",
                entry->d_name, entry->d_name, timeStr, (long long)fileStat.st_size);
            }
            if (S_ISDIR(fileStat.st_mode)) {
                sprintf(html_entry, "<a href=\"%s/\">%s/</a>                                            %s                   - \n",
                entry->d_name, entry->d_name, timeStr);
            }
        }
        if (size > strlen(html_entry)){
            strncat(response, html_entry, strlen(html_entry));
            size -= strlen(html_entry);
        } else {
            realloc(response, strlen(response) + 1001);
            size += 1000;
            strncat(response, html_entry, strlen(html_entry));
            size -= strlen(html_entry);
        }
        char tail[100] = "</pre><hr></body>\n </html>";
        if (size > strlen(tail)){
            strncat(response, tail, strlen(tail));
        } else {
            realloc(response, strlen(response) + 100);
            strncat(response, tail, strlen(tail));
        }
    }
    return response;
}



void handle_static(int client_fd, char* address){
    int file_fd = open(address, O_RDONLY);
    struct stat statbuf;
    fstat(file_fd, &statbuf);
    char* response;
    if (S_ISDIR(statbuf.st_mode)){
        response = get_directory(file_fd, address);
    }else {
        response = get_file(file_fd, statbuf, address);
    }
    send(client_fd, response, strlen(response) + 1, 0);
    free(response);
}