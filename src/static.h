#ifndef STATIC_FILE_H
#define STATIC_FILE_H

#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <dirent.h>
#include "config.h"
#include <unistd.h>
#include <time.h>

#define PATH_MAX 4096

#define FILE_HTTP_HEADER "HTTP/1.1 200 OK\n" \
        "Content-Type: application/octet-stream\n" \
        "Accept-Ranges: bytes\n" \
        "Cache-Control: private,max-age=0\n" \
        "Content-Disposition: attachment; filename=\"%s\" \n" \
        "Content-Length: %d\n" \
        "Connection: keep-alive\n" \
        "Etag: \"%s\"\n" \
        "Last-Modified: %s\n\n"

#define DIR_HTTP_HEADER "HTTP/1.1 200 OK \n" \
        "Content-Type: text/html; charset:utf-8 \n" \
        "Connection: close\n\n"

#define DIR_HTML_HEADER "<html> \n" \
                      "<head><title>Index of %s</title></head>\n" \
                    "<body> \n" \
                    "<h1>Index of %s</h1><hr><pre><a href=\"../\">../</a> \n"


char* handle_static(Route* route, char* url, const char *http_req, int* response_size);

#endif