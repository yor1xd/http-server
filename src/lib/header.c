#include "../../include/header.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

struct header_field {
  char* name;
  char* value;
};

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
  struct header_field *headers;
  unsigned int headers_count;
};

http_request create_header(){
  http_request req = {0};
  req.headers = malloc(HEADER_FIELD_CAPACITY * sizeof(struct header_field));
  if(req.headers == NULL) return req; //On error return empty
  req.headers_capacity = HEADER_FIELD_CAPACITY;

  return req;
}

size_t realloc_header(http_request *req){
  size_t new_capacity = req->headers_capacity * 2;
  struct header_field* new_headers = realloc(req->headers, new_capacity);
  if(new_headers == NULL) return -1;

  req->headers = new_headers;
  req->headers_capacity = new_capacity;

  return req->headers_capacity;
}

int add_header_field(http_request *req, char *name, char *value, size_t name_len, size_t value_len){
  if(req->headers_count + 1 > req->headers_capacity){
    int res = realloc_header(req);
    if(res == -1) return -1;
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
}

int add_header_method(http_request *req, char *method, size_t method_len){
  //Fuse all three together into add_header_info
  if(strcmp(method, "GET")) return -1; //Only support GET method

  char *header_method = strndup(method, method_len);
  if(header_method == NULL) return -1;

  req->method = header_method;

  return 0;
}

int add_header_path(http_request *req, char *path, size_t path_len){
  //Sanitize path first to use this function
  //Fuse all three together into add_header_info
  
  if(*path != '/') return -1; //Only accept absolute paths
  
  char *header_path = strndup(path, path_len);
  if(header_path == NULL) return -1;

  req->path = header_path;
  
  return 0;
}

int add_header_version(http_request *req, char *version, size_t version_len){
  //Fuse all three together into add_header_info
  char *header_version = strndup(version, version_len);
  if(header_version == NULL) return -1;

  req->version = header_version;

  return 0;
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

  int res = add_header_method(req, method, method_len);
  if(res == -1) return -1;

  char *path = space + 1;
  while(*path == ' ') path++;

  space = strchr(path, ' ');
  if(space == NULL) return -1;

  *space = '\0';
  size_t path_len = strlen(path);

  //Sanitize path now
  res = add_header_path(req, path, path_len);
  if(res == -1) return -1;

  char *version = space + 1;
  while(*version == ' ') version++;

  size_t version_len = strlen(version);

  res = add_header_version(req, version, version_len);
  if(res == -1) return -1;


  do {
    char *colon = strchr(header_field, ':');
    *colon = '\0';

    char *name = header_field;
    char *value = colon + 1;

    while(*value == ' ') value++;

    int res = add_header_field(req, name, value, strlen(name), strlen(value));
    if(res == -1) return -1; //Will yield malformed request, deal with it

    header_field = strtok(NULL, "\r\n");

  } while (header_field != NULL);

  return 0;

}
