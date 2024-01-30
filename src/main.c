#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include "config.h"
#include "defs.h"
#include "static.h"

int initsock(int port) {
    int server_fd;
    struct sockaddr_in server_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(1);
    }

    int true = 1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(1);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(1);
    }
    printf("server listening on port %d\n", port);

    return server_fd;
}

char* strremove(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        size_t size = 0;
        if ((p = strstr(p, sub)) != NULL) {
            size = strlen(p + len - 1) + 1;
            memmove(p, p + len -1, size);
        }
    }
    return str;
}

char *redirect_from_dest(const Route *route, char *http_req, int* response_size) {
    int dest_fd;
    struct sockaddr_in dest_addr;

    if ((dest_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("redirect from destination failed");
        return NULL;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(route->port);

    http_req = strremove(http_req, route->url);

    if (inet_pton(AF_INET, LOCALHOST, &dest_addr.sin_addr) <= 0) {
        perror("invalid address destination");
        return NULL;
    }

    int status;
    if ((status = connect(dest_fd,
                    (struct sockaddr*) &dest_addr,
                    sizeof(dest_addr)) < 0)) {
        perror("connection to destination failed");
        return NULL;
    }

    send(dest_fd, http_req, strlen(http_req), 0);

    char *buffer = (char*) malloc(BUFFER_SIZE * sizeof(char));
    int res;
    int pointer  =0;
    while(res = read(dest_fd, buffer + pointer, (BUFFER_SIZE - pointer) * sizeof(char)) > 0);

    if (res < 0) {
        perror("unable to read from destination");
        return NULL;
    }

    close(dest_fd);

    *response_size = strlen(buffer);
    return buffer;
}

char *get_response(int client_fd, char *buffer, int* response_size) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int res = getpeername(client_fd, (struct sockaddr*) &client_addr, &client_addr_len);

    regex_t regex, regex_with_slash;
    regcomp(&regex, "^GET /([^ ]*[^/]) HTTP/1", REG_EXTENDED);
    regcomp(&regex_with_slash, "^GET /([^ ]*/) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];
    int result1;
    if ((result1 = regexec(&regex, buffer, 2, matches, 0)) == 0 ||
         regexec(&regex_with_slash, buffer, 2, matches, 0) == 0) {
        char url[MAX_STR_LENGTH];
        int len = matches[1].rm_eo - matches[1].rm_so + 1;
        memcpy(url, (buffer +  matches[1].rm_so - 1), len);
        if (result1 == 0){
            url[len] = '/';
            len++;
        }
        url[len] = '\0';
        regfree(&regex);
        regfree(&regex_with_slash);
        Route *route = get_route(url);
        if (route == NULL){
            char* response = malloc(sizeof(HTTP_RESPONSE_NOT_FOUND));
            strncpy(response, HTTP_RESPONSE_NOT_FOUND, sizeof(HTTP_RESPONSE_NOT_FOUND));
            *response_size =  sizeof(HTTP_RESPONSE_NOT_FOUND);
            return response;
        }
        else if (route->port > 0) {
            return redirect_from_dest(route, buffer, response_size);
        } else if (route->static_path) {
            return handle_static(route, url, response_size);
        }
    }
    return NULL;
}

void *handle_req(void *arg) {
    int client_fd = *((int*) arg);
    char *buffer = (char*) malloc(BUFFER_SIZE * sizeof(char));
    char *response = NULL;
    int response_size = 0;

    ssize_t bytes_recvd = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_recvd > 0) {
        response = get_response(client_fd, buffer, &response_size);
        if (response)
            send(client_fd, response, response_size, 0);
        else
            send(client_fd, HTTP_RESPONSE_BAD_REQUEST, sizeof(HTTP_RESPONSE_BAD_REQUEST), 0);
    }
    close(client_fd);
    if (response)
        free(response);
    free(arg);
    free(buffer);
    return NULL;
}

int server_fd;
pthread_t threads[MAX_THREADS] = {0};
int cur_thread = 0;

void graceful_exit(int signum) {
    printf("graceful exit\n");
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i])
            pthread_cancel(threads[i]);
    }
    close(server_fd);
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, graceful_exit);
    read_config_file();
    print_all_config();  // Just for test
    int port = (argc > 1 ? atoi(argv[1]) : 8000);

    server_fd = initsock(port);

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));

        if ((*client_fd = accept(server_fd,
                            (struct sockaddr*) &client_addr,
                            &client_addr_len)) < 0) {
            perror("accept failed");
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_req, (void*) client_fd);
        pthread_detach(thread_id);
        threads[cur_thread++] = thread_id;
        cur_thread %= MAX_THREADS;
    }

    close(server_fd);
    return 0;
}