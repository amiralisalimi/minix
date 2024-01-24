#include <stdio.h>
#include <string.h>
#include "config.h"

Route routes[MAX_ROUTES];
int number_of_routes = 0;

int ispref(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

int read_config_file() {
    FILE *file = fopen("config.txt", "r");

    // Create a new empty config file if config.txt not found:
    if (file == NULL) {
        file = fopen("config.txt", "w");
        fclose(file);
        return 0;
    }

    char line[256], url[MAX_STR_LENGTH], path[MAX_STR_LENGTH];
    int have_errors = 0;

    // Read the config file line by line:
    while (fgets(line, sizeof(line), file)) {

        // Ignore commented lines:
        if (line[0] == '#')
            continue;

        // Reset the 3 parameter:
        int port = 0;
        url[0] = '\0';
        path[0] = '\0';

        // Extract the url, path and ports:
        int port_ret = sscanf(line, " %s -> %d", url, &port);
        if (port_ret != 2) {
            port_ret = sscanf(line, " %s -> %s", url, path);
            if (port_ret != 2) {
                printf("error in parsing line: %s\n", line);
                have_errors = 1;
                continue;
            }
        }

        // Store the route config in the routes array:
        Route route;
        route.port = port;
        memcpy(route.url, url, sizeof url);
        memcpy(route.static_path, path, sizeof path);
        routes[number_of_routes++] = route;
    }
    fclose(file);
    return have_errors;
}


void print_all_config() {
    for (int i = 0; i < number_of_routes; ++i) {
        printf("url: %s\t", routes[i].url);
        if (routes[i].port)
            printf("port: %d\n", routes[i].port);
        else
            printf("path: %s\n", routes[i].static_path);
    }
}
