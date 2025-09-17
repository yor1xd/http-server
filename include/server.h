#include "buffer.h"

int begin_server();
int accept_header(int socket_fd, header_buffer *header_buf);
size_t send_response(int conn_fd, char* buf);
