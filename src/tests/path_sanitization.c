#include "../../include/utils.h"
#include <stdio.h>
#include <string.h>

int main(){
  char *str = "/home/gabriel/../../home/gabriel/../gabriel/Downloads/\n";
  char *path = strndup(str, strlen(str)); //path pointer must be mutable, not const or literal read-only.

  sanitize_path(&path, strlen(path));

  printf("Sanitized path: %s", path);
  return 0;
}
