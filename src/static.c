#include "static.h"
void send_file(int client_fd, char* address){

    int file_fd = open(address, O_RDONLY);
    struct stat statbuf;
    fstat(file_fd, &statbuf);
    int fileLen = statbuf.st_size;
    char* filename = basename(address);
    char header[102400];
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
    send(client_fd, response, fileLen, 0);

    free(response);

}