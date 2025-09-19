#pragma once

#include "buffer.h"
#include "header.h"

int begin_server(char* port);
int accept_header(int socket_fd, header_buffer *header_buf);
size_t send_response(int conn_fd, char* buf, size_t buf_size);
int generate_response(http_request req, http_response res);
