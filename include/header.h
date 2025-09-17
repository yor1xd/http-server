#pragma once

#define HEADER_FIELD_CAPACITY 16
#define MAX_FIELD_SIZE 32

#include <stddef.h>
#include "buffer.h"

typedef struct http_request http_request;
typedef struct http_response http_response;

http_request create_header();
size_t realloc_header(http_request *req);

int parse_request(char* header_buf, http_request *req);
size_t load_response(struct http_response, char* response_buf);

int add_header_method(http_request *req, char *method, size_t method_len);
int add_header_path(http_request *req, char *path, size_t path_len);
int add_header_version(http_request *req, char *version, size_t version_len);

int add_header_field(http_request *req, char *name, char *value, size_t name_len, size_t value_len);
