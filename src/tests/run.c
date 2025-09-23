#include "../../include/server.h"

int main(){
  int socket_fd = begin_server("4444");
  int conn_fd;

  header_buffer *connection_buffer = create_buffer(BUFFER_SIZE);
  header_buffer *response_buffer = create_buffer(BUFFER_SIZE);

  http_request *req = create_request();
  http_response *res = create_response();

  for(;;){
    conn_fd = accept_header(socket_fd, connection_buffer);
    parse_request(get_buffer_content(connection_buffer), req);

    send_response(conn_fd, req, res, response_buffer);

    close_connection(conn_fd);
    clear_buffer(connection_buffer);
    clear_buffer(response_buffer);
    clear_response(res);
    clear_request(req);
  }

  return 0;
}
