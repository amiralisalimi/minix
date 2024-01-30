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

char* handle_static(Route* route, char* url, int* response_size);

#endif