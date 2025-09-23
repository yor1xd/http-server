#pragma once
#define BUFFER_SIZE 1024

//TODO: IFDEF

#include <stddef.h>

typedef struct header_buffer header_buffer;

header_buffer* create_buffer(size_t size);
size_t realloc_buffer(header_buffer *header_buf);
void append_buffer(header_buffer *header_buf, char* buf, size_t buf_size);
void free_buffer(header_buffer *header_buf);
int is_buffer_empty(header_buffer *header_buf);

void clear_buffer(header_buffer *header_buf);
void set_buffer(header_buffer *header_buf, char* buf, size_t buf_size);

char* get_buffer_content(header_buffer *header_buf);
size_t get_buffer_length(header_buffer *header_buf);
size_t get_buffer_capacity(header_buffer *header_buf);
