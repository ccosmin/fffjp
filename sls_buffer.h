#ifndef __SLS_BUFFER_H__
#define __SLS_BUFFER_H__

#include <stdio.h>

#include "ringbuf.h"

typedef struct sls_buffer sls_buffer;

struct sls_buffer
{
    ringbuf_t buf;
    size_t cap;
    FILE* file_input;
    char* str_input;
    int str_offset;
    size_t str_input_size;
};

sls_buffer* sls_buffer_create_from_file(const char* fn, size_t cap);
sls_buffer* sls_buffer_create_from_str(const char* str, size_t cap);

int sls_buffer_getc(sls_buffer* buffer);

int sls_buffer_peek(sls_buffer* buffer);

void sls_buffer_destroy(sls_buffer* buffer);

#endif

