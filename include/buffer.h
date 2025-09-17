#pragma once
#define BUFFER_SIZE 10

//TODO: IFDEF

#include <stddef.h>

typedef struct header_buffer header_buffer;

header_buffer* create_buffer(size_t size);
size_t realloc_buffer(header_buffer *header_buf);
void append_buffer(header_buffer *header_buf, char* buf, size_t buf_size);
void free_buffer(header_buffer *header_buf);
