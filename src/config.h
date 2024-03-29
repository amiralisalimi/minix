#include "defs.h"

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H


typedef struct Route {
    char url[MAX_STR_LENGTH];   // The url address
    char static_path[MAX_STR_LENGTH];  // The static directory path on the host machine
    int port;                   // The port number on the host machine
} Route;

int read_config_file();
void print_all_config();
Route *get_route(const char *url);

#endif //PROJECT_CONFIG_H
