#pragma once

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

typedef struct request {
    char method[8];
    char path[1024];
    char version[32];
    char host[256];
} request;

typedef struct response {
    unsigned int code;
    char* content_type;
    char** headers;
    char* body;

    ssize_t len_headers;

} response;

request* parseRequest(char* buf);

int beginServer();

void sendError(int fd);

ssize_t createResponse(struct response response, char* string);

ssize_t sendResponse(int fd, struct response response);
