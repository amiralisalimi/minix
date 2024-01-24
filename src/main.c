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

int initsock(int port) {
    int server_fd;
    struct sockaddr_in server_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(1);
    }

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

char *redirect_from_dest(const Route *route, const char *http_req) {
    int dest_fd;
    struct sockaddr_in dest_addr;

    if ((dest_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("redirect from destination failed");
        return NULL;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(route->port);

    if (inet_pton(AF_INET, LOCALHOST, &dest_addr.sin_addr) <= 0) {
        perror("invalid address destination");
        return NULL;
    }

    int status;
    if ((status = connect(dest_fd,
                    (struct sockaddr*) &dest_addr,
                    sizeof(dest_addr) < 0))) {
        perror("connection to destination failed");
        return NULL;
    }

    send(dest_fd, http_req, strlen(http_req), 0);

    char *buffer = (char*) malloc(BUFFER_SIZE * sizeof(char));
    int res = read(dest_fd, buffer, sizeof(buffer) - 1);

    if (res < 0) {
        perror("unable to read from destination");
        return NULL;
    }

    close(dest_fd);
    return buffer;
}

char *get_response(int client_fd, const char *buffer) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int res = getpeername(client_fd, (struct sockaddr*) &client_addr, &client_addr_len);

    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];

    if (regexec(&regex, buffer, 2, matches, 0) == 0) {
        char url[MAX_STR_LENGTH];
        strncpy(url, buffer + matches[1].rm_so - 1, matches[1].rm_eo - matches[1].rm_so + 1);
        Route *route = get_route(url);
        if (route->port > 0) {
            return redirect_from_dest(route, buffer);
        } else if (route->static_path) {
            // TODO: send file to http request
        }
    }
    return NULL;
}

void *handle_req(void *arg) {
    int client_fd = *((int*) arg);
    char *buffer = (char*) malloc(BUFFER_SIZE * sizeof(char));
    char *response = NULL;

    ssize_t bytes_recvd = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_recvd > 0) {
        response = get_response(client_fd, buffer);
        if (response)
            send(client_fd, response, sizeof(response), 0);
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