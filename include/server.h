#pragma once

#include "buffer.h"
#include "header.h"

int begin_server(char* port);
int accept_header(int socket_fd, header_buffer *header_buf);

size_t load_response(http_response* res, header_buffer *output_buf);
size_t send_response_buffer(int conn_fd, header_buffer *header_buf);

size_t send_response(int conn_fd, http_request *req, http_response *res, header_buffer *output_buf);

int generate_body(char* path, http_response *res);
int send_file(int conn_fd, int file_fd, http_response *res);

int generate_response(http_request *req, http_response *res);

int close_connection(int conn_fd);
