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

char* get_file(int file_fd, struct stat statbuf, char* address);
char* get_directory(int dir_fd, char* address);
char* handle_static(Route* route, char* url);

#endif