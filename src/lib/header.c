#include "../../include/header.h"
#include "../../include/buffer.h"

#include <stdlib.h>
#include <string.h>

//Isnt this better to be opaque?
//struct header_field {
//  char* name;
//  char* value;
//};

struct http_request {
  char* method;
  char* path;
  char* version;

  struct header_field *headers;

  size_t headers_count;
  size_t headers_capacity;
};

struct http_response {
  int code;

  struct header_buffer* body;
  struct header_field *headers;

  size_t headers_count;
  size_t headers_capacity;
};

http_request* create_request(){
  http_request *req = malloc(sizeof(http_request));
  if(req == NULL) return NULL;

  req->headers = malloc(HEADER_FIELD_CAPACITY * sizeof(struct header_field));
  if(req->headers == NULL) return NULL;

  req->headers_capacity = HEADER_FIELD_CAPACITY;
  return req;
}

size_t realloc_request(http_request *req){
  size_t new_capacity = req->headers_capacity * 2;
  struct header_field* new_headers = realloc(req->headers, new_capacity);
  if(new_headers == NULL) return -1;

  req->headers = new_headers;
  req->headers_capacity = new_capacity;

  return req->headers_capacity;
}

int add_request_info(http_request *req, enum HEADER_FIELDS type, char *name, char *value, size_t name_len, size_t value_len){
  switch (type) {
  case METHOD:
    if(name != NULL || name_len != 0) return -1;
    if(strcmp(value, "GET")) return -1; //Only support GET method

    char *header_method = strndup(value, value_len);
    if(header_method == NULL) return -1;

    req->method = header_method;
    return 0;

  case PATH:
    if(name != NULL || name_len != 0) return -1;
    if(*value != '/') return -1; //Only accept absolute paths
    
    char *header_path = strndup(value, value_len);
    if(header_path == NULL) return -1;

    req->path = header_path;
    return 0;

  case VERSION:
    if(name != NULL || name_len != 0) return -1;
    char *header_version = strndup(value, value_len);
    if(header_version == NULL) return -1;

    req->version = header_version;
    return 0;

  case HEADER_FIELD:
    if(req->headers_count + 1 > req->headers_capacity){
      if(realloc_request(req) == -1)
        return -1;
    }

    if(name_len > MAX_FIELD_SIZE || 
      value_len > MAX_FIELD_SIZE) return -1;
    //Implement errno to notify errors upstream with more detail

    char *field_name = strndup(name, name_len);
    if(field_name == NULL) return -1;

    char *field_value = strndup(value, value_len);
    if(field_value == NULL) return -1;

    req->headers[req->headers_count].name = field_name;
    req->headers[req->headers_count].value = field_value;

    req->headers_count++;
    return 0;

  default:
    return -1;
  }
}

int parse_request(char* header_buf, http_request *req){
  //Maybe deal with global http_request? as to avoid allocation on create and realloc
  //And maybe need to free it here first, so that heap does not get fragmented by past requests
  char* header_field;
  header_field = strtok(header_buf, "\r\n");
  if(header_field == NULL) return -1;

  char* space = strchr(header_field, ' ');
  if(space == NULL) return -1;

  *space = '\0';
  char *method = header_field;
  size_t method_len = strlen(method);

  int res = add_request_info(req, METHOD, NULL, method, 0, method_len);
  if(res == -1) return -1;

  char *path = space + 1;
  while(*path == ' ') path++;

  space = strchr(path, ' ');
  if(space == NULL) return -1;

  *space = '\0';
  size_t path_len = strlen(path);
  char* sanitized_path = realpath(path, NULL); //Can fail if path or file doesnt exist

  res = add_request_info(req, PATH, NULL, path, 0, path_len);
  if(res == -1) return -1;

  char *version = space + 1;
  while(*version == ' ') version++;

  size_t version_len = strlen(version);

  res = add_request_info(req, VERSION, NULL, version, 0, version_len);
  if(res == -1) return -1;

  header_field = strtok(NULL, "\r\n");
  if(header_field == NULL) return 0;

  do {
    char *colon = strchr(header_field, ':');
    if(colon == NULL) break;

    *colon = '\0';

    char *name = header_field;
    char *value = colon + 1;

    while(*value == ' ') value++;

    int res = add_request_info(req, HEADER_FIELD, name, value, strlen(name), strlen(value));
    if(res == -1) return -1; //Will yield malformed request, deal with it

    header_field = strtok(NULL, "\r\n");

  } while (header_field != NULL);

  return 0;
}

char* get_request_path(http_request *req){
  return req->path;
}

http_response* create_response(){
  http_response *res = malloc(sizeof(http_response));
  if(res == NULL) return NULL;

  res->headers = malloc(HEADER_FIELD_CAPACITY * sizeof(struct header_field));
  if(res->headers == NULL){
    free(res);
    return NULL;
  };

  res->headers_capacity = HEADER_FIELD_CAPACITY;

  res->body = create_buffer(BUFFER_SIZE); //SWITCH TO BODY_MAX_LEN
  if(res->body == NULL){
    free(res->headers);
    free(res);
    return NULL;
  }

  return res;
}

int add_response_header(http_response *res, char *name, char *value, size_t name_len, size_t value_len){
  if(res->headers_count + 1 > res->headers_capacity){
    int result = realloc_response(res);
    if(result == -1) return -1;
  }

  if(name_len > MAX_FIELD_SIZE || 
    value_len > MAX_FIELD_SIZE) return -1;
  //Implement errno to notify errors upstream with more detail

  char *field_name = strndup(name, name_len);
  if(field_name == NULL) return -1;

  char *field_value = strndup(value, value_len);
  if(field_value == NULL) return -1;

  res->headers[res->headers_count].name = field_name;
  res->headers[res->headers_count].value = field_value;

  res->headers_count++;

  return 0;
}

size_t realloc_response(http_response *res){
  size_t new_capacity = res->headers_capacity * 2;
  struct header_field* new_headers = realloc(res->headers, new_capacity);
  if(new_headers == NULL) return -1;

  res->headers = new_headers;
  res->headers_capacity = new_capacity;

  return res->headers_capacity;
}

header_buffer* get_body_buffer(http_response *res){
  return res->body;
}

void free_response(http_response *res){
  free(res->headers);
  free_buffer(res->body);
  free(res);
}

void free_request(http_request *req){
  free(req->headers);

  free(req->method);
  free(req->path);
  free(req->version);

  free(req);
}

int add_response_code(http_response *res, int code){ //No reason to be int function tho?
  res->code = code;
  return 0;
}

int get_response_code(http_response *res){
  return res->code;
}

size_t get_headers_count(http_response *res){
  return res->headers_count;
}

struct header_field* get_headers(http_response *res){
  return res->headers;
}

void clear_request(http_request *req){
  free(req->path);
  req->path = NULL;

  free(req->method);
  req->method = NULL;

  free(req->version);
  req->version = NULL;

  memset(req->headers, 0, req->headers_count * sizeof(struct header_field));
  req->headers_count = 0;
}

void clear_response(http_response *res){
  res->code = 0;
  clear_buffer(res->body);
  memset(res->headers, 0, res->headers_count * sizeof(struct header_field));
  res->headers_count = 0;
}
