#include "../../include/utils.h"
#include <string.h>

int simplify_path(char **path){
  if(*path == NULL) return -1;

  char *traversed = *path;

  if(strlen(traversed) == 1 || strlen(traversed) == 0) return 0;

  for(int i = strlen(traversed) - 2; i > 0; i--){ //2 to account for null-terminator and '/' character
    if(traversed[i] != '/') traversed[i] = '\0';
    else break;
  }

  return 0;
}

int sanitize_path(char **path, size_t path_len){ //Find a way to block immutable string literals
  if(*path == NULL || path_len == 0) return -1;
  char *path_str = *path;

  char *sanitized_path = malloc(path_len); //Sanitized path will be at max as long as the path length
  if(sanitized_path == NULL) return -1;

  sanitized_path[0] = '/';
  sanitized_path[1] = '\0';

  char *token = strtok(path_str, "/");
  while(token != NULL){
    if(strcmp(token, "..")){
      strncat(sanitized_path, token, strlen(token));
      if(sanitized_path[strlen(sanitized_path) - 1] != '/')
        strcat(sanitized_path, "/");

    } else {
      simplify_path(&sanitized_path);
    }

    token = strtok(NULL, "/");
  }

  free(path_str);
  *path = sanitized_path;

  return 0;
}

