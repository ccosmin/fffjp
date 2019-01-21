#include <stdlib.h>
#include <string.h>
#include "sls_buffer.h"
#include "ringbuf.h"

#define SCRATCH_BUFFER_SIZE 4096

sls_buffer* sls_buffer_create_from_file(const char* fn, size_t cap)
{
    sls_buffer* buf = (sls_buffer*)malloc(sizeof(sls_buffer));
    buf->buf = ringbuf_new(cap);
    buf->file_input = fopen(fn, "r");
    buf->str_input = NULL;
    buf->str_input_size = 0;

    return buf;
}

sls_buffer* sls_buffer_create_from_str(const char* str, size_t cap)
{
    sls_buffer* buf = (sls_buffer*)malloc(sizeof(sls_buffer));
    buf->buf = ringbuf_new(cap);
    buf->file_input = NULL;
    buf->str_input = strdup(str);
    buf->str_offset = 0;
    buf->str_input_size = strlen(str);

    return buf;
}

static int fill_buffer(sls_buffer* buffer)
{
    char scratch[SCRATCH_BUFFER_SIZE] = { 0 };
    if ( ringbuf_is_empty(buffer->buf) )
    {
        int buf_used = ringbuf_bytes_used(buffer->buf);
        int buf_cap = ringbuf_capacity(buffer->buf);
        int buf_free = ringbuf_bytes_free(buffer->buf);
        if ( buffer->file_input )
        {
            if ( feof(buffer->file_input) )
                return EOF;
            int read_ahead = buf_cap > SCRATCH_BUFFER_SIZE 
                ? SCRATCH_BUFFER_SIZE 
                : buf_cap;
            int ret = fread(scratch, 1, read_ahead, buffer->file_input); 
            if ( ! ret )
                return ret;
            ringbuf_memcpy_into(buffer->buf, scratch, ret);
        }
        else
        {
            if ( buffer->str_input_size == 0 )
                return EOF;
            int to_copy = buffer->str_input_size > buf_free ? buf_free: buffer->str_input_size;
            ringbuf_memcpy_into(buffer->buf, buffer->str_input + buffer->str_offset, to_copy);
            if ( to_copy < buffer->str_input_size )
            {
                buffer->str_input_size -= to_copy;
            }
            else
            {
                buffer->str_input_size = 0;
            }
            buffer->str_offset += to_copy;
        }
    }	
    return 1;
}

int sls_buffer_getc(sls_buffer* buffer)
{
    int fb_res = fill_buffer(buffer);
    if ( fb_res == 0 || fb_res == EOF )
        return fb_res; 
    int ret_char = *(char*)ringbuf_tail(buffer->buf); 
    ringbuf_advance(buffer->buf);
    return ret_char;
}

int sls_buffer_peek(sls_buffer* buffer)
{
    int fb_res = fill_buffer(buffer);
    if ( fb_res == 0 || fb_res == EOF )
        return fb_res; 
    int ret_char = *(char*)ringbuf_tail(buffer->buf); 
    return ret_char;
}

void sls_buffer_destroy(sls_buffer* buffer)
{
    if ( buffer->file_input ) 
    {
        fclose(buffer->file_input);
    }
    free(buffer->str_input);
    ringbuf_free(&buffer->buf);
    free(buffer);
}
