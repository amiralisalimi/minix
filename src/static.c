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
        "Content-Type: text/html; charset:utf-8 \n"
        "Connection: close\n\n");
    char *response = (char*) malloc(strlen(header) + 1000);
    strncpy(response, header, strlen(header));
    sprintf(response + strlen(header), "<html> \n"
                      "<head><title>Index of %s</title></head>\n"
                    "<body> \n"
                    "<h1>Index of %s</h1><hr><pre><a href=\"../\">../</a> \n", address, address);

    int size = strlen(header) + 1000 - sizeof(response);

    struct dirent *entry;
    DIR *dir = opendir(address);
    while ((entry = readdir(dir)) != NULL) {
        if(entry->d_name[0] == '.') continue;
        char *html_entry = (char*) malloc(1000);
        char filePath[PATH_MAX];
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
            response = realloc(response, strlen(response) + 1001);
            size += 1000;
            strncat(response, html_entry, strlen(html_entry));
            size -= strlen(html_entry);
        }
    }
    char tail[100] = "</pre><hr></body>\n </html>\n";
    if (size > strlen(tail)){
        strncat(response, tail, strlen(tail));
    } else {
        response = realloc(response, strlen(response) + 100);
        strncat(response, tail, strlen(tail));
    }
    return response;
}

void get_file_path(char* url, Route* route, char* address){
    snprintf(address, PATH_MAX, "%s%s", route->static_path, url + strlen(route->url));
}

char* handle_static(Route* route, char* url){
    char address[PATH_MAX];
    get_file_path(url, route, address);
    int file_fd;
    if (access(address, F_OK) == 0) {
        file_fd = open(address, O_RDONLY);
    }else{
        address[strlen(address) -1] = '\0';
        if (access(address, F_OK) == 0) 
            file_fd = open(address, O_RDONLY);
        else{
            char* response = malloc(sizeof(HTTP_RESPONSE_NOT_FOUND));
            strncpy(response, HTTP_RESPONSE_NOT_FOUND, sizeof(HTTP_RESPONSE_NOT_FOUND));
            return response;
        }
    }
    
    struct stat statbuf;
    fstat(file_fd, &statbuf);
    char* response;
    if (S_ISDIR(statbuf.st_mode)){
        response = get_directory(file_fd, address);
    }else {
        response = get_file(file_fd, statbuf, address);
    }
    return response;
}