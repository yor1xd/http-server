#include "../../include/server.h"
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

int accept_header(int socket_fd, header_buffer *header_buf){
  //Use global header buffer to avoid allocation with every connection
  char temp_buf[BUFFER_SIZE];
  int conn_fd;
  int bytes_received;

  conn_fd = accept(socket_fd, NULL, NULL);
  if(conn_fd == -1) return -1;

  while((bytes_received = recv(conn_fd, temp_buf, BUFFER_SIZE, 0)) > 0){
    append_buffer(header_buf, temp_buf, bytes_received);
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

size_t send_response(int conn_fd, char* buf, size_t buf_size){
  size_t total_bytes = 0, bytes_sent;
  const char *buffer = buf;

  while(total_bytes < buf_size){
    bytes_sent = send(conn_fd, buffer + total_bytes, buf_size - total_bytes, 0);

    if(bytes_sent == -1){
      if(errno == EINTR) continue;
      else return -1;
    };

    if(bytes_sent == 0) break;

    total_bytes += bytes_sent;
  }

  return total_bytes;
}
