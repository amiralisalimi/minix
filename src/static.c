#include "static.h"

void generate_file_headers(char *header, const char *filename, const struct stat fstat) {
    struct tm last_modified = *gmtime(&fstat.st_mtime);
    char lmstr[100];
    strftime(lmstr, sizeof(lmstr), "%a, %d %b %Y %H:%M:%S %Z", &last_modified);
    char etag[100];
    snprintf(etag, sizeof(etag), "%lx-%x", (unsigned long) fstat.st_mtime, (unsigned) fstat.st_size);
    snprintf(header, 1000, FILE_HTTP_HEADER, filename, (int) fstat.st_size, etag, lmstr);
}

char* get_file(int file_fd, struct stat statbuf, char* address, int* response_size){
    FILE *fp = fopen(address, "r");

    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char * file_buf = malloc(file_size);
    fread(file_buf, 1, file_size, fp);

    char* filename = basename(address);

    char header[1000];
    generate_file_headers(header, filename, statbuf);

    *response_size = file_size + strlen(header);
    char *response = (char*) malloc(*response_size);
    memcpy(response, header, strlen(header));
    memcpy(response + strlen(header), file_buf, file_size);

    free(file_buf);
    return response;
}

char* get_directory(int dir_fd, char* address){
    char header[300];
    snprintf(header, strlen(DIR_HTTP_HEADER) + 1, DIR_HTTP_HEADER);
    char *response = (char*) malloc(strlen(header) + 10000);
    strncpy(response, header, strlen(header));
    snprintf(response + strlen(header), 10000, DIR_HTML_HEADER, address, address);

    int size = 10000 - sizeof(response);

    struct dirent *entry;
    DIR *dir = opendir(address);
    while ((entry = readdir(dir)) != NULL) {
        if(entry->d_name[0] == '.') continue;
        char *html_entry = (char*) malloc(5000);
        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), "%s/%s", address, entry->d_name);
        struct stat fileStat;
        if (stat(filePath, &fileStat) == 0) {
            char timeStr[20];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_mtime));
            if (S_ISREG(fileStat.st_mode)) {
                snprintf(html_entry, 5000, "<a href=\"%s\">%s</a>                                            %s                   %lld \n",
                    entry->d_name, entry->d_name, timeStr, (long long)fileStat.st_size);
            }
            if (S_ISDIR(fileStat.st_mode)) {
                snprintf(html_entry, 5000, "<a href=\"%s/\">%s/</a>                                            %s                   - \n",
                    entry->d_name, entry->d_name, timeStr);
            }
        }
        if (size > strlen(html_entry)){
            strncat(response, html_entry, strlen(html_entry));
            size -= strlen(html_entry);
        } else {
            response = realloc(response, strlen(response) + strlen(html_entry) +  1001);
            size += 1000;
            strncat(response, html_entry, strlen(html_entry));
            size -= strlen(html_entry);
        }
        free(html_entry);
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

int check_not_modified(const char *http_req, struct stat fstat) {
    struct tm last_modified = *gmtime(&fstat.st_mtime);
    char lmstr[100];
    strftime(lmstr, sizeof(lmstr), "%a, %d %b %Y %H:%M:%S %Z", &last_modified);
    char etag[100];
    snprintf(etag, sizeof(etag), "\"%lx-%x\"", (unsigned long) fstat.st_mtime, (unsigned) fstat.st_size);
    char *header_modified_since, *header_none_match;
    if (header_none_match = strstr(http_req, HTTP_IF_NONE_MATCH_HEADER)) {
        header_none_match += strlen(HTTP_IF_NONE_MATCH_HEADER);
        return (strncmp(etag, header_none_match, strlen(etag)) == 0);
    }
    if (header_modified_since = strstr(http_req, HTTP_IF_MODIFIED_SINCE_HEADER)) {
        header_modified_since += strlen(HTTP_IF_MODIFIED_SINCE_HEADER);
        return (strncmp(lmstr, header_modified_since, strlen(lmstr)) == 0);
    }
    return 0;
}

char* handle_static(Route* route, char* url, const char *http_req, int* response_size){
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
            *response_size = strlen(HTTP_RESPONSE_NOT_FOUND);
            return response;
        }
    }
    
    struct stat statbuf;
    fstat(file_fd, &statbuf);
    char* response;
    if (S_ISDIR(statbuf.st_mode)){
        response = get_directory(file_fd, address);
        *response_size = strlen(response);
#if USE_STATIC_CACHE
    } else if (check_not_modified(http_req, statbuf)) {
        response = malloc(sizeof(HTTP_RESPONSE_NOT_MODIFIED));
        strncpy(response, HTTP_RESPONSE_NOT_MODIFIED, sizeof(HTTP_RESPONSE_NOT_MODIFIED));
        *response_size = strlen(HTTP_RESPONSE_NOT_MODIFIED);
#endif
    } else {
        response = get_file(file_fd, statbuf, address, response_size);
    }
    return response;
}