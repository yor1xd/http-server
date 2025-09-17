#include "../../include/server.h"
#include <sys/socket.h>

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
