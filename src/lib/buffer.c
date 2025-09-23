#include "../../include/buffer.h"
#include <stdlib.h>
#include <string.h>

struct header_buffer {
  char* buf;
  size_t length;
  size_t capacity;
};

header_buffer* create_buffer(size_t size){
  header_buffer *header_buf = malloc(sizeof(header_buffer));

  header_buf->capacity = size;
  header_buf->buf = malloc(size);
  header_buf->length = 0;

  return header_buf;
}

size_t realloc_buffer(header_buffer *header_buf){
  size_t new_size = header_buf->capacity * 2;
  char* new_buf = realloc(header_buf->buf, new_size);
  if(new_buf == NULL) return -1;

  header_buf->buf = new_buf;
  header_buf->capacity = new_size;

  return header_buf->capacity;
}

void append_buffer(header_buffer *header_buf, char* buf, size_t buf_size){
  while(header_buf->length + buf_size > header_buf->capacity){
    realloc_buffer(header_buf); //Deal with -1 error here, maybe break or upstream it.
  }

  memcpy(&header_buf->buf[header_buf->length], buf, buf_size);
  header_buf->length += buf_size;

}

void free_buffer(header_buffer *header_buf){
  free(header_buf->buf);
  free(header_buf);

}

int is_buffer_empty(header_buffer *header_buf){
  if(header_buf->length == 0) return 1;
  else return 0;
}

void clear_buffer(header_buffer *header_buf){
  memset(header_buf->buf, 0, header_buf->length);
  header_buf->length = 0;
}

void set_buffer(header_buffer *header_buf, char* buf, size_t buf_size){
  clear_buffer(header_buf);
  append_buffer(header_buf, buf, buf_size);
}

char* get_buffer_content(header_buffer *header_buf){
  return header_buf->buf;
}

size_t get_buffer_length(header_buffer *header_buf){
  return header_buf->length;
}

size_t get_buffer_capacity(header_buffer *header_buf){
  return header_buf->capacity;
}
