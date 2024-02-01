#ifndef PROJECT_DEFS_H
#define PROJECT_DEFS_H

#define USE_PTHREAD 1
#define MAX_ROUTES  20
#define MAX_THREADS 100
#define MAX_STR_LENGTH 100
#define BUFFER_SIZE (1 << 20)

#define LOCALHOST "127.0.0.1"
#define HTTP_RESPONSE_BAD_REQUEST "HTTP 400 Bad Request"
#define HTTP_RESPONSE_NOT_FOUND "HTTP/1.1 404 Not Found \n Content-Type: text/html \n\n <html> \n<head><title>404 Not Found</title></head> \n<body> \n<center><h1>404 Not Found</h1></center>\n</body>\n</html>"
#define HTTP_RESPONSE_NOT_MODIFIED "HTTP/1.1 304 Not Modified"

#define HTTP_IF_MODIFIED_SINCE_HEADER "If-Modified-Since: "
#define HTTP_IF_NONE_MATCH_HEADER "If-None-Match: "

#endif //PROJECT_DEFS_H
