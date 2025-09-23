#pragma once

#define HEADER_FIELD_CAPACITY 16
#define MAX_FIELD_SIZE 128

#include <stddef.h>
#include "buffer.h"

typedef struct http_request http_request;
typedef struct http_response http_response;

struct header_field {
  char* name;
  char* value;
};

enum HEADER_FIELDS {
  METHOD,
  PATH,
  VERSION,
  HEADER_FIELD,
};

http_request* create_request();
int parse_request(char* header_buf, http_request *req);
int add_request_info(http_request *req, enum HEADER_FIELDS type, char *name, char *value, size_t name_len, size_t value_len);
char* get_request_path(http_request *req);
void clear_request(http_request *req);
void free_request(http_request *req);

http_response* create_response();
header_buffer* get_body_buffer(http_response *res);
int get_response_code(http_response *res);
size_t get_headers_count(http_response *res);
struct header_field* get_headers(http_response *res);
size_t realloc_response(http_response *res);
int add_response_code(http_response *res, int code);
int add_response_header(http_response *res, char *name, char *value, size_t name_len, size_t value_len);
void free_response(http_response *res);
void clear_response(http_response *res);





