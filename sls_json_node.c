#include "sls_json_node.h"
#include "sls_buffer.h"
#include <string.h>
#include <stdlib.h>

typedef struct sls_json_node_context
{
    sls_json_node* root;
    sls_json_node* current;
    sls_json_node_array* array;
    sls_json_node_kv* kvs;
    size_t size;
    const char* last_key;
};

static int error(void* context, const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
}

static int event_key(const char* key_name, void* context)
{
    sls_json_node_context* c = (sls_json_node_context*)context;
    c->last_key = strdup(key_name);
}

static int event_value(sls_json_value_type type, 
                       const sls_json_value* value, 
                       void* context)
{
}

static int event_start_object(void* context)
{
}

static int event_end_object(void* context)
{
    sls_json_node_context* c = (sls_json_node_context*)context;
}

static int event_start_array(void* context)
{
    sls_json_node_context* c = (sls_json_node_context*)context;
    c->array = (sls_json_node_array*)malloc(sizeof(sls_json_node_array)); 
}

static int event_end_array(void* context)
{
}

sls_json_node* sls_json_node_parse(sls_buffer* buffer)
{
    sls_json_node_context context = { 0 };

    sls_json_callback cb = { 0 }; 
    cb.error = &error;

    int res = sls_parse(buffer, &cb, &context);
    sls_buffer_destroy(b);
    return res;
}

