#include "../../include/server.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>

int accept_header(int socket_fd, header_buffer *header_buf){
  char temp_buf[BUFFER_SIZE];
  int conn_fd, bytes_received;

  conn_fd = accept(socket_fd, NULL, NULL);
  if(conn_fd == -1) return -1;

  while((bytes_received = recv(conn_fd, temp_buf, BUFFER_SIZE, 0)) != 0){
    if(bytes_received == -1){
      if(errno == EINTR) continue;
      else return -1;
    }

    append_buffer(header_buf, temp_buf, bytes_received);
    char *buffer = get_buffer_content(header_buf);
    if(buffer == NULL) return -1;

    if(strstr(buffer, "\r\n\r\n") != NULL) break; //End of header
  }

  return conn_fd;
};

int begin_server(char* port){
  int res, socket_fd;
  struct addrinfo hints = {0}, *list, *result;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  res = getaddrinfo(NULL, port, &hints, &list);
  if(res) return -1;

  for(result = list; result != NULL; result = result->ai_next){
    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(socket_fd == -1){
        continue;
    }

    res = bind(socket_fd, result->ai_addr, result->ai_addrlen);
    if(res == 0) break;

    close(socket_fd);
  }

  freeaddrinfo(list);

  if(result == NULL) return -1;

  res = listen(socket_fd, 0); //No more than one connection
  if(res == -1) return -1;

  return socket_fd;
}

size_t send_response_buffer(int conn_fd, header_buffer *header_buf){ //Always check total_bytes against header_buf length
  size_t total_bytes = 0, bytes_sent;

  const char *buffer = get_buffer_content(header_buf);
  if(buffer == NULL) return 0;

  const size_t buffer_size = get_buffer_length(header_buf);

  while(total_bytes < buffer_size){
    bytes_sent = send(conn_fd, buffer + total_bytes, buffer_size - total_bytes, 0);

    if(bytes_sent == -1){
      if(errno == EINTR) continue;
      else return 0;
    };

    if(bytes_sent == 0) break;

    total_bytes += bytes_sent;
  }

  return total_bytes;
}

int generate_body(char *path, http_response *res){
  header_buffer *res_body = get_body_buffer(res);
  if(res_body == NULL) return -1;

  DIR* dir = opendir(path);
  if(dir == NULL) return -1; //send_error must go within send_response() and have signature send_error(conn_fd, ERR_ENUM);
  //ERR_ENUM = {NOT_PERMITTED, NOT_FOUND, PATH_EXCEEDED, etc.}

  char *cwd = malloc(PATH_MAX);
  if(cwd == NULL) return -1;
  strcpy(cwd, path); //Path does not surpass PATH_MAX, so its okay to strcpy instead of strncpy
  size_t cwd_len = strlen(cwd);

  struct dirent* dirent;

  char* href = malloc(PATH_MAX);
  if(href == NULL) return -1;

  append_buffer(res_body, "<body>", 6); //That's ugly.
  while((dirent = readdir(dir)) != NULL){
    cwd[cwd_len] = '\0';

    if(cwd_len + strlen(dirent->d_name) + 1 > PATH_MAX) return -1; //send_error(conn_fd, PATH_EXCEEDED);
    strcat(cwd, dirent->d_name);
    
    //Add trailing slash to directory
    if(dirent->d_type == DT_DIR
    && cwd[strlen(cwd) - 1] != '/')
        strcat(cwd, "/");

    //send_error(conn_fd, MALFORMED_HTML);
    if((snprintf(href, PATH_MAX, "<a href=\"%s\">%s</a><br>", cwd, dirent->d_name)) >= PATH_MAX) return -1;

    append_buffer(res_body, href, strlen(href));
  }
  append_buffer(res_body, "</body>", 7);

  free(cwd);
  free(href);
  return 0;
}

int send_file(int conn_fd, int file_fd, http_response *res){
  header_buffer *file_buf = create_buffer(BUFFER_SIZE);

  struct stat file_stat = {0};
  int result = fstat(file_fd, &file_stat);
  if(result == -1){
    free_buffer(file_buf);
    return -1;
  };

  result = add_response_header(res, "Content-Type", "application/octet-stream", 12, 24);
  if(result == -1){
    free_buffer(file_buf);
    return -1;
  };

  char *file_size = malloc(MAX_FIELD_SIZE);
  if(file_size == NULL){
    free_buffer(file_buf);
    return -1;
  }

  //Converts file size to string
  if((snprintf(file_size, MAX_FIELD_SIZE, "%lu", file_stat.st_size)) >= MAX_FIELD_SIZE){
    free_buffer(file_buf);
    return -1;
  }

  result = add_response_header(res, "Content-Length", file_size, 14, strlen(file_size));
  if(result == -1){
    free_buffer(file_buf);
    free(file_size);
    return -1;
  };

  free(file_size);

  result = load_response(res, file_buf);
  if(result == -1){
    free_buffer(file_buf);
    return -1;
  };

  result = send_response_buffer(conn_fd, file_buf);
  if(result == -1){
    free_buffer(file_buf);
    return -1;
  }

  off_t offset = 0;
  size_t bytes_sent = 0, total_bytes = 0;

  while((bytes_sent = sendfile(conn_fd, file_fd, &offset, file_stat.st_size)) > 0){
    if(bytes_sent == -1) return -1;

    total_bytes += bytes_sent;
  };

  return (total_bytes == file_stat.st_size) ? 0 : -1;
}

size_t load_response(http_response* res, header_buffer *output_buf){
  if(!is_buffer_empty(output_buf)) 
    clear_buffer(output_buf);

  char* header_field = malloc(MAX_FIELD_SIZE);
  if(header_field == NULL) return -1;

  if((snprintf(header_field, MAX_FIELD_SIZE, "HTTP/1.1 %d\r\n", get_response_code(res)) >= MAX_FIELD_SIZE)){
    return -1;
  }

  append_buffer(output_buf, header_field, strlen(header_field));

  struct header_field* headers = get_headers(res);
  if(headers == NULL) return -1;
  for(int i = 0; i < get_headers_count(res); i++){
    if((snprintf(header_field, MAX_FIELD_SIZE, "%s: %s\r\n", headers[i].name, headers[i].value)) >= MAX_FIELD_SIZE){
      return -1;
    }

    append_buffer(output_buf, header_field, strlen(header_field));
  }

  append_buffer(output_buf, "\r\n", 2);

  header_buffer* body = get_body_buffer(res);
  if(body == NULL) return -1;
  append_buffer(output_buf, get_buffer_content(body), get_buffer_length(body));

  free(header_field);
  
  return get_buffer_length(output_buf);
}

size_t send_response(int conn_fd, http_request *req, http_response *res, header_buffer *output_buf){
  char *path = get_request_path(req);
  int result = generate_body(path, res);

  if(result == -1){
    if(errno == ENOTDIR){
      int file_fd = openat(0, path, O_RDONLY);
      if(file_fd == -1) return -1; //send_error(NOT_FOUND);
      
      add_response_code(res, 200);
      if(send_file(conn_fd, file_fd, res) == -1) return -1; //send_error(DOWNLOAD_FAILED)

      return 0;
    } else return -1; //send_error(SERVER_ISSUE);
  }

  add_response_header(res, "Content-Type", "text/html", 12, 9);

  size_t buffer_size = load_response(res, output_buf);
  size_t bytes_sent = send_response_buffer(conn_fd, output_buf);
  if(bytes_sent != buffer_size) return -1;

  return 0;
}

int close_connection(int conn_fd){
  close(conn_fd);
  return 0;
}
