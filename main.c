#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE (1 << 20)

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

void *handle_req(void *arg) {
    int client_fd = *((int*) arg);
    char *buffer = (char*) malloc(BUFFER_SIZE * sizeof(char));
    
    ssize_t bytes_recvd = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_recvd > 0) {
        char *response = (char*) malloc(BUFFER_SIZE * sizeof(char));
        strcpy(response, "Hello, World!");
        send(client_fd, response, sizeof(response), 0);
        free(response);
    }
    close(client_fd);
    free(arg);
    free(buffer);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = (argc > 1 ? atoi(argv[1]) : 8000);
    int server_fd = initsock(port);

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
    }

    close(server_fd);
    return 0;
}